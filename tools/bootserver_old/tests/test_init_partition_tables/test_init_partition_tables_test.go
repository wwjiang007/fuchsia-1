// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"testing"

	bootservertest "go.fuchsia.dev/fuchsia/tools/bootserver_old/tests"
)

func TestInitPartitionTables(t *testing.T) {
	_ = bootservertest.StartQemu(t, "netsvc.all-features=true, netsvc.netboot=true", "full")

	logPattern := []bootservertest.LogMatch{
		{"Received request from ", true},
		{"Proceeding with nodename ", true},
		{"Transfer starts", true},
		{"Transfer ends successfully", true},
		{"Issued reboot command to", false},
	}

	bootservertest.CmdSearchLog(
		t, logPattern,
		bootservertest.ToolPath("bootserver"), "-n", bootservertest.DefaultNodename,
		"--init-partition-tables", "/dev/class/block/000", "-1", "--fail-fast")
}
