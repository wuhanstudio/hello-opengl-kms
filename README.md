# KMS GLSL

A minimal example of OpenGL fragment shaders, using the [DRM/KMS Linux kernel subsystem](https://en.wikipedia.org/wiki/Direct_Rendering_Manager).

It runs shaders fullscreen, and does not require any windowing system, like X or Wayland.

## Build

```shell
$ sudo apt install gcc make
$ sudo apt install libdrm-dev libgbm-dev libegl-dev libgles2-mesa-dev
$ sudo apt install libxcb-randr0-dev
$ make

$ ./main examples/costal_landscape.glsl
```

## Credits

The DRM/KMS ceremony code is copied from [kmscube](https://gitlab.freedesktop.org/mesa/kmscube/).

The shader examples are copied from the [Shadertoy](https://www.shadertoy.com) website URLs commented at the top of each file.
