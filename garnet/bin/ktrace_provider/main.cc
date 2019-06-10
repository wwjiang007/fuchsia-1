// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/async-loop/cpp/loop.h>
// TODO(PT-127): Replace the next line with trace-provider/provider.h.
#include <trace-provider/tracelink_provider.h>

#include "garnet/bin/ktrace_provider/app.h"
#include "src/lib/fxl/command_line.h"
#include "src/lib/fxl/log_settings_command_line.h"

using namespace ktrace_provider;

int main(int argc, const char** argv) {
  auto command_line = fxl::CommandLineFromArgcArgv(argc, argv);
  if (!fxl::SetLogSettingsFromCommandLine(command_line))
    return 1;

  async::Loop loop(&kAsyncLoopConfigAttachToThread);
  // TODO(PT-127): Use TraceProviderWithFdio.
  trace::TracelinkProviderWithFdio trace_provider(
      loop.dispatcher(), "ktrace_provider");

  App app(command_line);
  loop.Run();
  return 0;
}
