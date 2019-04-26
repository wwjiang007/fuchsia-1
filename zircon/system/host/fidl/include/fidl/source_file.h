// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ZIRCON_SYSTEM_HOST_FIDL_INCLUDE_FIDL_SOURCE_FILE_H_
#define ZIRCON_SYSTEM_HOST_FIDL_INCLUDE_FIDL_SOURCE_FILE_H_

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fidl {

class SourceFile {
public:
    SourceFile(std::string filename, std::string data);
    virtual ~SourceFile();

    std::string_view filename() const { return filename_; }
    std::string_view data() const { return data_; }

    // This is in the coordinates that most editors use. Lines start
    // at 1 but columns start at 0.
    struct Position {
        int line;
        int column;
    };

    virtual std::string_view LineContaining(std::string_view view, Position* position_out) const;

private:
    std::string filename_;
    std::string data_;
    std::vector<std::string_view> lines_;
};

} // namespace fidl

#endif // ZIRCON_SYSTEM_HOST_FIDL_INCLUDE_FIDL_SOURCE_FILE_H_
