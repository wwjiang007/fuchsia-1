// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//! Type-safe bindings for Zircon object properties.

use fuchsia_zircon_sys as sys;
use std::ops::Deref;

#[derive(Copy, Clone, Ord, PartialOrd, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct Property(sys::zx_obj_props_t);

impl Deref for Property {
    type Target = sys::zx_obj_props_t;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

/// A definition for properties about a zircon object.
///
///  # Safety
///
/// `PROPERTY` must correspond to a valid `zx_obj_props_t` property and `PropTy`
/// must be a type that can be safely replaced with the byte representation of
/// the associated `zx_object_props_t` value. Additionally `PropTy` must be
/// a type that is safe to drop when uninitialized, i.e. it must be a PoD type.
pub unsafe trait PropertyQuery {
    /// The raw `Property` value
    const PROPERTY: Property;
    /// The data type of this property
    type PropTy;
}

/// Indicates that it is valid to get the property for an object.
pub unsafe trait PropertyQueryGet : PropertyQuery {}
/// Indicates that it is valid to set the property for an object.
pub unsafe trait PropertyQuerySet : PropertyQuery {}

assoc_values!(Property, [
    NONE =  sys::ZX_OBJ_PROP_NONE;
    WAITABLE = sys::ZX_OBJ_PROP_WAITABLE;
    NAME = sys::ZX_PROP_NAME;

    #[cfg(target_arch = "x86_64")]
    REGISTER_GS = sys::ZX_PROP_REGISTER_GS;
    #[cfg(target_arch = "x86_64")]
    REGISTER_FS = sys::ZX_PROP_REGISTER_FS;

    PROCESS_DEBUG_ADDR = sys::ZX_PROP_PROCESS_DEBUG_ADDR;
    PROCESS_VDSO_BASE_ADDRESS = sys::ZX_PROP_PROCESS_VDSO_BASE_ADDRESS;
    SOCKET_RX_BUF_MAX = sys::ZX_PROP_SOCKET_RX_BUF_MAX;
    SOCKET_RX_BUF_SIZE = sys::ZX_PROP_SOCKET_RX_BUF_SIZE;
    SOCKET_TX_BUF_MAX = sys::ZX_PROP_SOCKET_TX_BUF_MAX;
    SOCKET_TX_BUF_SIZE = sys::ZX_PROP_SOCKET_TX_BUF_SIZE;
    SOCKET_RX_THRESHOLD = sys::ZX_PROP_SOCKET_RX_THRESHOLD;
    SOCKET_TX_THRESHOLD = sys::ZX_PROP_SOCKET_TX_THRESHOLD;
    CHANNEL_TX_MSG_MAX = sys::ZX_PROP_CHANNEL_TX_MSG_MAX;
    JOB_KILL_ON_OOM = sys::ZX_PROP_JOB_KILL_ON_OOM;
]);
