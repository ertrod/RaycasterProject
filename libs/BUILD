load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

cc_import(
    name = "sdl2lib",
    interface_library = "SDL2/lib/x64/SDL2.lib",
    shared_library = "SDL2/lib/x64/SDL2.dll",

)
cc_import(
    name = "sdl2mainlib",
    interface_library = "SDL2/lib/x64/SDL2main.lib",
    shared_library = "SDL2/lib/x64/SDL2.dll",
)
cc_import(
    name="sdl2image",
    interface_library = "SDL2_image/lib/x64/SDL2_image.lib",
    shared_library = "SDL2_image/lib/x64/SDL2_image.dll",
)
cc_import(
    name="sdl2mixer",
    interface_library = "SDL2_mixer/lib/x64/SDL2_mixer.lib",
    shared_library = "SDL2_mixer/lib/x64/SDL2_mixer.dll",
)
cc_import(
    name="sdl2ttf",
    interface_library = "SDL2_ttf/lib/x64/SDL2_ttf.lib",
    shared_library = "SDL2_ttf/lib/x64/SDL2_ttf.dll",
)

cc_library(
    name = "sdl2",
    hdrs = glob([
        "SDL2/include/*.h",
        "SDL2_image/include/*.h",
        "SDL2_mixer/include/*.h",
        "SDL2_ttf/include/*.h",
    ]),
    includes = ["SDL2/include"],
    deps = [
        "sdl2lib",
        "sdl2mainlib",
        "sdl2image",
        "sdl2mixer",
        "sdl2ttf",
    ],
    linkopts = ["-DEFAULTLIB:shell32.lib"],
    visibility = ["//visibility:public"],
)
