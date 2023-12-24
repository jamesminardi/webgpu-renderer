# WebGPU C++ Template

> [!WARNING]
> Work in Progress

TODO:
- Hello Triangle
- Build from command-line
- Build using emscripten, is it a dependency or requirement?

## About

This is a WebGPU C++ 

### Dependencies
- [`GLFW`](https://github.com/glfw/glfw): Window creation
- [`WEBGPU`](https://github.com/eliemichel/WebGPU-distribution): WebGPU distribution
  - [`WGPU-Native`](https://github.com/gfx-rs/wgpu-native): Native interface to [`wgpu`](https://github.com/gfx-rs/wgpu) Rust library developed for Firefox
  - [`Dawn`](https://dawn.googlesource.com/dawn): Google's WebGPU implementation developed for Chrome in C++
  - [`Emscripten`](https://github.com/emscripten-core/emscripten): Webassembly support
- [`WebGPU-Cpp`](https://github.com/eliemichel/WebGPU-Cpp): WebGPU C++ Wrapper

*All of these are included through git submodules.

For more information on the different platforms, 

## Setup
> [!WARNING]
> TODO: Untested


### Build Requirements
- [`cmake`](https://cmake.org) 3.20+
- a modern C++17 compiler (`gcc-8`, `clang-6.0`, `MSVC 2017` or above)

### Build via Command-Line
- Clone this project using git.
- From the root of this project update all the submodules with `git submodule update --init --recursive`.
- Follow the command-line instructions below:

  ```
  cmake . -B build/release ${OPTIONS}
  cmake --build build/release
  ```

#### ${OPTIONS}
Use `-G ${GENERATOR}` to use a specific [build system](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) (e.g. `"Unix Makefiles"`, `"Visual Studio 17 2022"`, etc.).

Use `-D WEBGPU_BACKEND=${BACKEND}` to use a specific WebGPU platform (e.g. `DAWN` or `WGPU`). Defaults to `WGPU`.

#### Run
Execute either `./build/app` (linux/macOS/MinGW) or `build/Debug/app.exe` (MSVC).

### Build via CMake Compatible IDE (CLion)

> [!NOTE]  
> On Windows I was unable to build WebGPU-Dawn using any toolchain but Visual Studio. MinGW and Cygwin could not track some specific Windows SDK headers (e.g. DirectX Compiler). WSL is unsupported as far as I can tell as well. YMMV

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


## Project Structure
- `build`: Generated build files and binary application.
- `data`: Promotional Data like screenshots.
- `external`: External dependencies like git submodules.
- `resources`: Project resources such as textures, sounds, music, etc.
- `src`: Source code