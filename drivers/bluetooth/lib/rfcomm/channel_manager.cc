// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/async/cpp/task.h>
#include <lib/async/default.h>

#include "garnet/drivers/bluetooth/lib/l2cap/l2cap.h"
#include "garnet/drivers/bluetooth/lib/rfcomm/channel_manager.h"
#include "garnet/drivers/bluetooth/lib/rfcomm/session.h"

namespace btlib {
namespace rfcomm {

std::unique_ptr<ChannelManager> ChannelManager::Create(l2cap::L2CAP* l2cap) {
  FXL_DCHECK(l2cap);
  // ChannelManager constructor private; can't use make_unique.
  auto channel_manager =
      std::unique_ptr<ChannelManager>(new ChannelManager(l2cap));
  if (l2cap->RegisterService(
          l2cap::kRFCOMM,
          [cm = channel_manager->weak_ptr_factory_.GetWeakPtr()](
              auto l2cap_channel) {
            if (cm) {
              if (cm->RegisterL2CAPChannel(l2cap_channel)) {
                FXL_LOG(INFO)
                    << "rfcomm: Registered incoming channel with handle "
                    << l2cap_channel->link_handle();
              } else {
                FXL_LOG(WARNING)
                    << "rfcomm: Failed to register incoming channel with"
                    << " handle " << l2cap_channel->link_handle();
              }
            }
          },
          async_get_default_dispatcher())) {
    return channel_manager;
  }

  return nullptr;
}

bool ChannelManager::RegisterL2CAPChannel(
    fbl::RefPtr<l2cap::Channel> l2cap_channel) {
  auto handle = l2cap_channel->link_handle();

  if (handle_to_session_.find(handle) != handle_to_session_.end()) {
    FXL_LOG(WARNING) << "Handle " << handle << " already registered";
    return false;
  }

  auto session = Session::Create(
      l2cap_channel, fit::bind_member(this, &ChannelManager::ChannelOpened));
  if (!session) {
    FXL_LOG(ERROR) << "Couldn't start a session on the given L2CAP channel";
    return false;
  }
  handle_to_session_[handle] = std::move(session);
  return true;
}

void ChannelManager::OpenRemoteChannel(hci::ConnectionHandle handle,
                                       ServerChannel server_channel,
                                       ChannelOpenedCallback channel_opened_cb,
                                       async_dispatcher_t* dispatcher) {
  FXL_DCHECK(channel_opened_cb);
  FXL_DCHECK(dispatcher);

  auto session_it = handle_to_session_.find(handle);

  if (session_it == handle_to_session_.end()) {
    l2cap_->OpenChannel(
        handle, l2cap::kRFCOMM,
        [this, handle, server_channel, dispatcher,
         cb = std::move(channel_opened_cb)](auto l2cap_channel) mutable {
          if (!l2cap_channel) {
            FXL_LOG(ERROR) << "rfcomm: Failed to open L2CAP channel with"
                           << " handle " << handle;
            async::PostTask(dispatcher, [cb_ = std::move(cb)] {
              cb_(nullptr, kInvalidServerChannel);
            });
            return;
          }

          FXL_LOG(INFO) << "rfcomm: opened L2CAP session with handle "
                        << handle;

          FXL_DCHECK(handle_to_session_.find(handle) ==
                     handle_to_session_.end());
          handle_to_session_.emplace(
              handle,
              Session::Create(
                  l2cap_channel,
                  fbl::BindMember(this, &ChannelManager::ChannelOpened)));

          // Re-run OpenRemoteChannel now that the session is opened.
          async::PostTask(dispatcher_,
                          [this, handle, server_channel, dispatcher,
                           cb_ = std::move(cb)]() mutable {
                            OpenRemoteChannel(handle, server_channel,
                                              std::move(cb_), dispatcher);
                          });
        },
        dispatcher_);
    return;
  }

  FXL_DCHECK(session_it != handle_to_session_.end());

  session_it->second->OpenRemoteChannel(
      server_channel, [cb = std::move(channel_opened_cb), dispatcher](
                          auto rfcomm_channel, auto server_channel) mutable {
        async::PostTask(dispatcher, [cb_ = std::move(cb), rfcomm_channel,
                                     server_channel]() {
          cb_(rfcomm_channel, server_channel);
        });
      });
}

ServerChannel ChannelManager::AllocateLocalChannel(
    ChannelOpenedCallback cb, async_dispatcher_t* dispatcher) {
  // Find the first free Server Channel and allocate it.
  for (ServerChannel server_channel = kMinServerChannel;
       server_channel <= kMaxServerChannel; ++server_channel) {
    if (server_channels_.find(server_channel) == server_channels_.end()) {
      server_channels_[server_channel] =
          std::make_pair(std::move(cb), dispatcher);
      return server_channel;
    }
  }

  return kInvalidServerChannel;
}

ChannelManager::ChannelManager(l2cap::L2CAP* l2cap)
    : dispatcher_(async_get_default_dispatcher()),
      l2cap_(l2cap),
      weak_ptr_factory_(this) {
  FXL_DCHECK(l2cap_);
}

void ChannelManager::ChannelOpened(fbl::RefPtr<Channel> rfcomm_channel,
                                   ServerChannel server_channel) {
  auto server_channel_it = server_channels_.find(server_channel);
  FXL_DCHECK(server_channel_it != server_channels_.end())
      << "rfcomm: New channel created on an unallocated Server Channel";

  async::PostTask(server_channel_it->second.second,
                  [server_channel, rfcomm_channel,
                   cb = server_channel_it->second.first.share()]() {
                    cb(rfcomm_channel, server_channel);
                  });
}

}  // namespace rfcomm
}  // namespace btlib
