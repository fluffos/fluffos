extern crate built;
extern crate cmake;

use std::path::PathBuf;
use std::{env, fs};

fn main() {
    let p = cmake::Config::new(".")
        .always_configure(true)
        .build();

    println!("cargo:rustc-link-search=native={}/lib", p.display());
    println!("cargo:rustc-link-lib=dylib=driver");

    let dst = PathBuf::from(env::var("OUT_DIR").unwrap());
    let filename = format!("{}driver{}", env::consts::DLL_PREFIX, env::consts::DLL_SUFFIX);
    fs::copy(
        format!("{}/lib/{}", p.display(), filename),
        dst.join(format!("../../../{}", filename)),
    )
    .unwrap();

    built::write_built_file().expect("Failed to acquire build-time information");
}
