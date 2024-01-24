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
  name="sdl2imagelib",
  interface_library = "SDL2_image/lib/x64/SDL2_image.lib",
  shared_library = "SDL2_image/lib/x64/SDL2_image.dll",
)
cc_import(
  name="sdl2mixerlib",
  interface_library = "SDL2_mixer/lib/x64/SDL2_mixer.lib",
  shared_library = "SDL2_mixer/lib/x64/SDL2_mixer.dll",
)
cc_import(
  name="sdl2ttflib",
  interface_library = "SDL2_ttf/lib/x64/SDL2_ttf.lib",
  shared_library = "SDL2_ttf/lib/x64/SDL2_ttf.dll",
)

cc_library(
  name = "sdl2",
  hdrs = glob([
    "SDL2/include/*.h",
  ]),
  includes = ["SDL2/include"],
  deps = [
    "sdl2lib",
    "sdl2mainlib",
  ],
  linkopts = ["-DEFAULTLIB:shell32.lib"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "sdl2image",
  hdrs = glob([
    "SDL2_image/include/*.h",
  ]),
  includes = ["SDL2_image/include"],
  deps = [
    "sdl2imagelib",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "sdl2mixer",
  hdrs = glob([
    "SDL2_mixer/include/*.h",
  ]),
  includes = ["SDL2_mixer/include"],
  deps = [
    "sdl2mixerlib",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "sdl2ttf",
  hdrs = glob([
    "SDL2_ttf/include/*.h",
  ]),
  includes = ["SDL2_ttf/include"],
  deps = [
    "sdl2ttflib",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "sdl",
  deps = [
    "sdl2",
    "sdl2image",
    "sdl2mixer",
    "sdl2ttf",
  ],
  visibility = ["//visibility:public"],
)
