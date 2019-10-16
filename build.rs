extern crate built;
extern crate cmake;

use std::path::PathBuf;
use std::{env, fs};

fn main() {
    let p = cmake::Config::new(".").always_configure(true).build();

    println!("cargo:rustc-cdylib-link-arg=-Wl,-z,origin,-rpath=$ORIGIN");
    println!("cargo:rustc-link-search=native={}/lib", p.display());
    println!("cargo:rustc-link-lib=dylib=driver");

    let dst = PathBuf::from(env::var("OUT_DIR").unwrap());
    fs::copy(
        format!("{}/lib/libdriver.so", p.display()),
        dst.join("../../../libdriver.so"),
    )
    .unwrap();

    built::write_built_file().expect("Failed to acquire build-time information");
}
