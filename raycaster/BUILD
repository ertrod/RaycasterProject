load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [
        "//libs:sdl2",
        "//SDL2wrapper:SDL2wrapper",
    ],
    data = [
        "data/fonts/Vera.ttf",
        "data/wolftextures.png",
    ]
)