// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use {argh::FromArgs, ffx_core::ffx_command};

#[ffx_command()]
#[derive(FromArgs, Debug, PartialEq)]
#[argh(subcommand, name = "status", description = "Status of the target device.")]
pub struct TargetStatus {
    /// display descriptions of entries.
    #[argh(switch)]
    pub desc: bool,

    /// display label of entries (default for json).
    #[argh(switch)]
    pub label: bool,

    /// generate machine readable output (JSON).
    #[argh(switch)]
    pub json: bool,
}
