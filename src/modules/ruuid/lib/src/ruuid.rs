extern crate libc;
extern crate uuid;

use std::{ptr, ffi::CStr, slice};
use libc::{c_char, c_int, size_t};
use uuid::Uuid;

/// UUID namespace for SIP as defined in RFC 7989.
const NAMESPACE_SIP: Uuid = uuid::uuid!("a58587da-c93d-11e2-ae90-f4ea67801e29");

/// Generate a new nil `Uuid` with all zeros.
///
/// # Safety
///
/// Make sure you destroy the Uuid with [`ruuid_destroy()`] once you are
/// done with it.
///
/// [`ruuid_destroy()`]: fn.ruuid_destroy.html
#[no_mangle]
pub unsafe extern "C" fn ruuid_generate_nil() -> *mut Uuid {
    let uuid = Uuid::nil();
    Box::into_raw(Box::new(uuid))
}

/// Generate a new `Uuid` of version 4 with random data.
///
/// # Safety
///
/// Make sure you destroy the Uuid with [`ruuid_destroy()`] once you are
/// done with it.
///
/// [`ruuid_destroy()`]: fn.ruuid_destroy.html
#[no_mangle]
pub unsafe extern "C" fn ruuid_generate_version_4() -> *mut Uuid {
    let uuid = Uuid::new_v4();
    Box::into_raw(Box::new(uuid))
}

/// Generate a new `Uuid` of version 5 with the SIP namespace and given name.
///
/// # Safety
///
/// Make sure you destroy the Uuid with [`ruuid_destroy()`] once you are
/// done with it.
///
/// [`ruuid_destroy()`]: fn.ruuid_destroy.html
#[no_mangle]
pub unsafe extern "C" fn ruuid_generate_version_5_sip(name: *const c_char) -> *mut Uuid {
    if name.is_null() {
        return ptr::null_mut();
    }
    let raw = CStr::from_ptr(name);

    let uuid = Uuid::new_v5(&NAMESPACE_SIP, raw.to_bytes());
    Box::into_raw(Box::new(uuid))
}

/// Parses a `Uuid` from given string.
///
/// # Safety
///
/// Make sure you destroy the Uuid with [`ruuid_destroy()`] once you are
/// done with it.
///
/// [`ruuid_destroy()`]: fn.ruuid_destroy.html
#[no_mangle]
pub unsafe extern "C" fn ruuid_parse(uuid: *const c_char) -> *mut Uuid {
    if uuid.is_null() {
        return ptr::null_mut();
    }

    let raw = CStr::from_ptr(uuid);

    let uuid_as_str = match raw.to_str() {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };

    let uuid = match Uuid::try_parse(&uuid_as_str) {
        Ok(s) => s,
        Err(_) => return ptr::null_mut(),
    };

    Box::into_raw(Box::new(uuid))
}

/// Test a `Uuid` whether it's a nil UUID.
#[no_mangle]
pub unsafe extern "C" fn ruuid_is_nil(uuid: *const Uuid) -> c_int {
    if uuid.is_null() {
        return -1;
    }

    let uuid = &*uuid;

    if uuid.is_nil() {
        return 1;
    }

    return 0;
}

/// Copy the uuid in simple form into a user-provided buffer, returning the number of
/// bytes copied.
///
/// If an error is encountered, this returns `-1`.
#[no_mangle]
pub unsafe extern "C" fn ruuid_get_simple(uuid: *const Uuid, buffer: *mut c_char, length: size_t) -> c_int {
    if uuid.is_null() || buffer.is_null() {
        return -1;
    }

    let uuid = &*uuid;
    let buffer: &mut [u8] = slice::from_raw_parts_mut(buffer as *mut u8,
                                                      length as usize);

    let string = uuid.simple().to_string();

    if buffer.len() < string.len() {
        return -1;
    }

    ptr::copy_nonoverlapping(string.as_ptr(), buffer.as_mut_ptr(), string.len());

    string.len() as c_int
}

/// Copy the uuid in hyphenated form into a user-provided buffer, returning the number of
/// bytes copied.
///
/// If an error is encountered, this returns `-1`.
#[no_mangle]
pub unsafe extern "C" fn ruuid_get_hyphenated(uuid: *const Uuid, buffer: *mut c_char, length: size_t) -> c_int {
    if uuid.is_null() || buffer.is_null() {
        return -1;
    }

    let uuid = &*uuid;
    let buffer: &mut [u8] = slice::from_raw_parts_mut(buffer as *mut u8,
                                                      length as usize);

    let string = uuid.hyphenated().to_string();

    if buffer.len() < string.len() {
        return -1;
    }

    ptr::copy_nonoverlapping(string.as_ptr(), buffer.as_mut_ptr(), string.len());

    string.len() as c_int
}

/// Copy the uuid in urn form into a user-provided buffer, returning the number of
/// bytes copied.
///
/// If an error is encountered, this returns `-1`.
#[no_mangle]
pub unsafe extern "C" fn ruuid_get_urn(uuid: *const Uuid, buffer: *mut c_char, length: size_t) -> c_int {
    if uuid.is_null() || buffer.is_null() {
        return -1;
    }

    let uuid = &*uuid;
    let buffer: &mut [u8] = slice::from_raw_parts_mut(buffer as *mut u8,
                                                      length as usize);

    let string = uuid.urn().to_string();

    if buffer.len() < string.len() {
        return -1;
    }

    ptr::copy_nonoverlapping(string.as_ptr(), buffer.as_mut_ptr(), string.len());

    string.len() as c_int
}

/// Copy the uuid in braced form into a user-provided buffer, returning the number of
/// bytes copied.
///
/// If an error is encountered, this returns `-1`.
#[no_mangle]
pub unsafe extern "C" fn ruuid_get_braced(uuid: *const Uuid, buffer: *mut c_char, length: size_t) -> c_int {
    if uuid.is_null() || buffer.is_null() {
        return -1;
    }

    let uuid = &*uuid;
    let buffer: &mut [u8] = slice::from_raw_parts_mut(buffer as *mut u8,
                                                      length as usize);

    let string = uuid.braced().to_string();

    if buffer.len() < string.len() {
        return -1;
    }

    ptr::copy_nonoverlapping(string.as_ptr(), buffer.as_mut_ptr(), string.len());

    string.len() as c_int
}

/// Destroy a `Uuid` once you are done with it.
#[no_mangle]
pub unsafe extern "C" fn ruuid_destroy(uuid: *mut Uuid) {
    if !uuid.is_null() {
        drop(Box::from_raw(uuid));
    }
}