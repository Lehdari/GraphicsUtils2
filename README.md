GraphicsUtils2
==============

This toolkit is intended to eventually serve as the successor for https://github.com/Lehdari/GraphicsUtils.
Long way to go still, Vulkan is not learnt overnight.

Setup
-----

1) Clone the repository

2) Clone submodules:
    ```
    cd GraphicsUtils2/
    git submodule update --init --recursive
    ```

3) Install LunarG Vulkan SDK: https://vulkan.lunarg.com/sdk/home

4) Install Eigen: https://eigen.tuxfamily.org/index.php?title=Main_Page
    
    On debian-based Linux distros (such as Ubuntu):
    ```
    sudo apt install libeigen3-dev
    ```
5) Install CMake >=3.27

Build
-----

```
mkdir build
cd build/
cmake .. -GNinja
ninja -j0
```

Run Demos
---------

In `build/` directory:
```
src/demos/demo_vulkan_triangle
```

Run Tests
---------

In `build/` directory:
```
ninja test
```
