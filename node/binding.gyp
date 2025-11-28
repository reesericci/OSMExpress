{
  "targets": [
    {
      "target_name": "osmx_native",
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "cflags_cc": ["-std=c++14", "-fexceptions"],
      "sources": [
        "src/binding.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "../include",
        "../capnpc_generated/include",
        "../_deps/osmium-src/include",
        "../_deps/protozero-src/include",
        "../_deps/capnproto-src/c++/src",
        "../_deps/capnproto-build/c++/src",
        "../_deps/roaring-src/include",
        "../_deps/roaring-src/cpp",
        "../_deps/roaring-build/src",
        "../vendor/s2geometry/src",
        "/opt/homebrew/opt/lmdb/include",
        "/opt/homebrew/include",
        "/usr/local/include"
      ],
      "libraries": [
        "<(module_root_dir)/../libosmx-static.a",
        "<(module_root_dir)/../_deps/capnproto-build/c++/src/capnp/libcapnp.a",
        "<(module_root_dir)/../_deps/capnproto-build/c++/src/kj/libkj.a",
        "<(module_root_dir)/../_deps/roaring-build/src/libroaring.a",
        "<(module_root_dir)/../vendor/s2geometry/libs2.a",
        "-L/opt/homebrew/opt/lmdb/lib",
        "-L/opt/homebrew/lib",
        "-L/usr/local/lib",
        "-llmdb",
        "-lbz2",
        "-lz",
        "-lexpat"
      ],
      "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
      "conditions": [
        ["OS=='mac'", {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
            "CLANG_CXX_LIBRARY": "libc++",
            "MACOSX_DEPLOYMENT_TARGET": "10.15",
            "OTHER_CPLUSPLUSFLAGS": ["-std=c++14", "-Wno-deprecated-declarations"]
          }
        }],
        ["OS=='linux'", {
          "cflags_cc": ["-std=c++14", "-fPIC"]
        }]
      ]
    }
  ]
}
