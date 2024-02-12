# WebGPU C++ Terrain Renderer

> [!WARNING]
> Work in Progress
> Building to web currently unsupported.


## About

This is a WIP in-depth exploration of terrain generation that I am developing in C++ using the [WebGPU](https://www.w3.org/TR/webgpu) graphics API.

## Roadmap / Topics
1. Camera Controller (zoom, click to drag, etc.) :white_check_mark:
2. Dev GUI :white_check_mark:
3. Generate plane meshes of configurable sizes :white_check_mark:
4. Adjust mesh using noise as a heightmap
5. Explore Value, Perlin, Simplex, and Cubic noise varieties. :white_check_mark:
6. Sample multiple noises at once (Fractional Brownian Motion) :white_check_mark:
7. Adjust noises individually including scale, amplitude, etc.
8. Color map the terrain based on height, steepness, etc.
9. Dynamically load and unload chunks of terrain
10. Dynamically adjust level of detail a quadtree, clip maps, etc.
11. Cull the terrain using frustum and occlusionS
12. Post-processing effects, atmosphere, lighting, skybox, etc.
15. Vegetation
16. Water


### Dependencies
- [`GLFW`](https://github.com/glfw/glfw): Window creation
- [`GLFW3 WebGPU`](https://github.com/eliemichel/WebGPU-Cpp): Wrapper for GLFW to support cross-platform WebGPU
- [`WEBGPU`](https://github.com/eliemichel/WebGPU-distribution): WebGPU distribution
  - [`WGPU-Native`](https://github.com/gfx-rs/wgpu-native): Native interface to [`wgpu`](https://github.com/gfx-rs/wgpu) Rust library developed for Firefox
  - [`Dawn`](https://dawn.googlesource.com/dawn): Google's WebGPU implementation developed for Chrome in C++
- [`WebGPU-Cpp`](https://github.com/eliemichel/WebGPU-Cpp): WebGPU C++ Wrapper
- [`ImGui`](https://github.com/ocornut/imgui): Lightweight dev gui
- [`GLM`](https://github.com/g-truc/glm): Math library
- [`STB`](): Lightweight image loading/saving (Included)

_All of these are available through git submodules. Instructions are below._

---

## Setup

### Build Requirements
- [`cmake`](https://cmake.org): 3.20+
- a modern C++20 compiler: todo update versions:(`gcc-8`, `clang-6.0`, `MSVC 2017` or above)
- [`Emscripten`](https://github.com/emscripten-core/emscripten): (Required for building to web)

> [!NOTE]  
> On Windows I was unable to build and run using any build system except MSVC. MinGW and Cygwin could not track some specific Windows SDK headers (e.g. DirectX Compiler) among other issues. WSL is also unsupported as far as I can tell as well. YMMV

---

### Build via Command-Line
- Clone this project using git.
- From the root of this project update all the submodules with `git submodule update --init --recursive`.
- Follow the command-line instructions below:

  ```
  cmake . -B build ${OPTIONS}
  cmake --build build
  ```

#### ${OPTIONS}
Use `-G ${GENERATOR}` to use a specific [build system](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) (e.g. `"Unix Makefiles"`, `"Visual Studio 17 2022"`, etc.).

Use `-D WEBGPU_BACKEND=${BACKEND}` to use a specific WebGPU platform (e.g. `DAWN` or `WGPU`). Defaults to `WGPU`.

#### Run
Execute either `./build/app` (linux/macOS/MinGW) or `build/Debug/app.exe` (MSVC).

---

### Build via CMake Compatible IDE (CLion)

- Clone this project using git.
- From the root of this project update all the submodules with `git submodule update --init --recursive`
- Open the project in an editor that directly supports CMAKE files.

Within `Build, Execution, Deployment` -> `Toolchains`, drag `Visual Studio` to the top of the list to make it the default option.

![readme_toolchains.png](data/readme_toolchains.png)


Next, navigate to `Build, Execution, Deployment` -> `CMake`, selecting your corresponding toolchain and generator. Append to the `CMake options:` field: `-D WEBGPU_BACKEND=${BACKEND}` where `${BACKEND}` is either `WGPU` or `DAWN`.

> [!TIP]  
> A second CMake profile may be created to build to both platforms.

![readme_cmake.png](data/readme_cmake.png)

Reload your CMake project.

---

## Project Structure
- `build`: Generated build files and binary application.
- `data`: Screenshots and other data.
- `docs`: Personal notes,documentation, and examples.
- `external`: External dependencies (e.g. git submodules).
- `resources`: Project resources such as textures, sounds, music, etc.
- `src`: Source code

---
