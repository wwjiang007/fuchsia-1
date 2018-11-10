// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <stdint.h>
#include <unistd.h>

#include <cobalt-client/cpp/counter-internal.h>
#include <cobalt-client/cpp/types-internal.h>
#include <fbl/atomic.h>
#include <fbl/function.h>
#include <fbl/string.h>
#include <fbl/vector.h>
#include <lib/fidl/cpp/vector_view.h>

namespace cobalt_client {
namespace internal {

// Note: Everything on this namespace is internal, no external users should rely
// on the behaviour of any of these classes.

// Base class for histogram, that provides a thin layer over a collection of buckets
// that represent a histogram. Once constructed, unless moved, the class is thread-safe.
// All allocations happen when constructed.
//
// This class is not moveable, not copyable or assignable.
// This class is thread-compatible.
template <uint32_t num_buckets>
class BaseHistogram {
public:
    using Count = uint64_t;
    using Bucket = uint32_t;

    BaseHistogram() = default;
    BaseHistogram(const BaseHistogram&) = delete;
    BaseHistogram(BaseHistogram&&) = delete;
    BaseHistogram& operator=(const BaseHistogram&) = delete;
    BaseHistogram& operator=(BaseHistogram&&) = delete;
    ~BaseHistogram() = default;

    // Returns the number of buckets of this histogram.
    constexpr uint32_t size() const { return num_buckets; }

    void IncrementCount(Bucket bucket, Count val = 1) {
        ZX_DEBUG_ASSERT_MSG(bucket < size(), "IncrementCount bucket(%u) out of range(%u).", bucket,
                            size());
        buckets_[bucket].Increment(val);
    }

    Count GetCount(uint32_t bucket) const {
        ZX_DEBUG_ASSERT_MSG(bucket < size(), "GetCount bucket out of range.");
        return buckets_[bucket].Load();
    }

protected:
    // Counter for the abs frequency of every histogram bucket.
    BaseCounter<uint64_t> buckets_[num_buckets];
};

// This class provides a histogram which represents a full fledged cobalt metric. The histogram
// owner will call |Flush| which is meant to incrementally persist data to cobalt.
//
// This class is not moveable, copyable or assignable.
// This class is thread-compatible.
template <uint32_t num_buckets>
class RemoteHistogram : public BaseHistogram<num_buckets>, public FlushInterface {
public:
    RemoteHistogram() = delete;
    RemoteHistogram(const RemoteMetricInfo& metric_info)
        : BaseHistogram<num_buckets>(), metric_info_(metric_info) {
        for (uint32_t i = 0; i < num_buckets; ++i) {
            bucket_buffer_[i].count = 0;
            bucket_buffer_[i].index = i;
        }
        auto* buckets = buffer_.mutable_event_data();
        buckets->set_data(bucket_buffer_);
        buckets->set_count(num_buckets);
    }
    RemoteHistogram(const RemoteHistogram&) = delete;
    RemoteHistogram(RemoteHistogram&&) = delete;
    RemoteHistogram& operator=(const RemoteHistogram&) = delete;
    RemoteHistogram& operator=(RemoteHistogram&&) = delete;
    ~RemoteHistogram() override = default;

    FlushResult Flush(Logger* logger) override {
        if (!buffer_.TryBeginFlush()) {
            return FlushResult::kIgnored;
        }

        // Sets every bucket back to 0, not all buckets will be at the same instant, but
        // eventual consistency in the backend is good enough.
        for (uint32_t bucket_index = 0; bucket_index < num_buckets; ++bucket_index) {
            bucket_buffer_[bucket_index].count = this->buckets_[bucket_index].Exchange();
        }
        return logger->Log(metric_info_, bucket_buffer_, num_buckets) ? FlushResult::kSucess
                                                                      : FlushResult::kFailed;
    }

    void UndoFlush() override {
        ZX_DEBUG_ASSERT_MSG(buffer_.IsFlushing(),
                            "UndoFlush should not be called when no flush is ongoing.");

        for (uint32_t bucket_index = 0; bucket_index < num_buckets; ++bucket_index) {
            this->buckets_[bucket_index].Increment(bucket_buffer_[bucket_index].count);
        }
    }

    void CompleteFlush() override {
        ZX_DEBUG_ASSERT_MSG(buffer_.IsFlushing(),
                            "CompleteFlush should not be called when no flush is ongoing.");
        buffer_.CompleteFlush();
    }

    // Returns the metric_id associated with this remote metric.
    const RemoteMetricInfo& metric_info() const { return metric_info_; }

private:
    // Buffer for out of line allocation for the data being sent
    // through fidl. This buffer is rewritten on every flush, and contains
    // an entry for each bucket.
    HistogramBucket bucket_buffer_[num_buckets];

    EventBuffer<fidl::VectorView<HistogramBucket>> buffer_;

    // Metric information such as metric_id, event_code and component.
    RemoteMetricInfo metric_info_;
};

} // namespace internal
} // namespace cobalt_client
