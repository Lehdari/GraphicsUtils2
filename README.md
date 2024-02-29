# GraphicsUtils2

This toolkit is intended to eventually serve as the successor for https://github.com/Lehdari/GraphicsUtils.
Long way to go still, Vulkan is not learnt overnight.

## Setup

1) Clone the repository

2) Clone submodules (most of the dependencies are imported via them):
    ```
    cd GraphicsUtils2/
    git submodule update --init --recursive
    ```
3) Install CMake >=3.27

### Install the dependencies
For Ubuntu 22.04 an installation script is provided:
   ```
   scripts/install_dependencies_ubuntu_20_04.sh
   ```

Manual installation:

- Install LunarG Vulkan SDK: https://vulkan.lunarg.com/sdk/home
- For testing install GTest: `sudo apt install libgtest-dev` (on Ubuntu)

## Build

```
mkdir build
cd build/
cmake .. -GNinja [-DGU2_BUILD_DEMOS=ON] [-DGU2_BUILD_TESTS=ON]
ninja -j0
```
Building demos and tests are disabled by default.

### Selecting the Windowing backend

GraphicsUtils2 supports [SDL2](https://www.libsdl.org/) and [GLFW](https://www.glfw.org/)
libraries for providing the Vulkan-enabled window and user input functions. The backend
can be selected with `GU2_BACKEND` CMake setting.

## Run Demos

Passing `GU2_BUILD_DEMOS=ON` to cmake required.

In `build/` directory:
```
src/demos/demo_vulkan_triangle
src/demos/demo_vulkan_box
```

## Run Tests

Passing `GU2_BUILD_TESTS=ON` to cmake required.

In `build/` directory:
```
ninja test
```
