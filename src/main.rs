extern crate libc;

// Use of a mod or pub mod is not actually necessary.
pub mod built_info {
    // The file has been placed there by the build script.
    include!(concat!(env!("OUT_DIR"), "/built.rs"));
}

use libc::c_char;
use libc::c_int;

use std::ffi::CString;

fn print_built_info() {
    println!(
        "FluffOS-Rust: {} ({}/{})",
        built_info::PKG_NAME,
        built_info::TARGET,
        built_info::PROFILE
    );
    println!("Version: {:?}", built_info::GIT_VERSION);
    println!("Rust: {}", built_info::RUSTC_VERSION);
    println!("Built On: {}", built_info::BUILT_TIME_UTC);
}

extern "C" {
    fn driver_main(argc: c_int, argv: *const *const c_char);
}

fn main() {
    println!("========================================================================");
    print_built_info();

    // create a vector of zero terminated strings
    let args = std::env::args()
        .map(|arg| CString::new(arg).unwrap())
        .collect::<Vec<CString>>();
    // convert the strings to raw pointers
    let c_args = args
        .iter()
        .map(|arg| arg.as_ptr())
        .collect::<Vec<*const c_char>>();
    unsafe {
        // pass the pointer of the vector's internal buffer to a C function
        driver_main(c_args.len() as c_int, c_args.as_ptr());
    };
}
