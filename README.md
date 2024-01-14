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

## Build

```
mkdir build
cd build/
cmake .. -GNinja [-DGU2_BUILD_DEMOS=ON] [-DGU2_BUILD_TESTS=ON]
ninja -j0
```
Building demos and tests are disabled by default.

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
