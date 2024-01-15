# FLAMEGPU 2 Visualiser

This repository contains the components for the interactive, real-time visualiser for FLAME GPU 2.x.
This repository is automatically pulled in by [FLAMEGPU2](https://github.com/FLAMEGPU/FLAMEGPU2) when the `FLAMEGPU_VISUALISATION` option is enabled within the CMake configuration.
It is unlikely to be useful independently.

## Requirements

+ [CMake](https://cmake.org/download/) `>= 3.18`
+ [CUDA](https://developer.nvidia.com/cuda-downloads) `>= 11.0` and a Compute Capability `>= 3.5` NVIDIA GPU.
+ C++17 capable C++ compiler (host), compatible with the installed CUDA version
  + [Microsoft Visual Studio 2019](https://visualstudio.microsoft.com/) (Windows)
  + [make](https://www.gnu.org/software/make/) and [GCC](https://gcc.gnu.org/) `>= 8.1` or [Clang](https://clang.llvm.org/) `>= 9` (Linux)
+ [git](https://git-scm.com/)
+ [SDL](https://www.libsdl.org/)
+ [GLM](http://glm.g-truc.net/) *(consistent C++/GLSL vector maths functionality)*
+ [GLEW](http://glew.sourceforge.net/) *(GL extension loader)*
+ [FreeType](http://www.freetype.org/)  *(font loading)*
+ [DevIL](http://openil.sourceforge.net/)  *(image loading)*
+ [Fontconfig](https://www.fontconfig.org/)  *(Linux only, font detection)*

Optionally:

+ [cpplint](https://github.com/cpplint/cpplint) for linting code


## Building the Visualiser

The Visualiser uses [CMake](https://cmake.org/), as a cross-platform process, for configuring and generating build directives, e.g. `Makefile` or `.vcxproj`.
CMake can be configured from the command line or through a GUI application. Several editors / IDEs also provide CMake integration.

To build the visualiser under linux in Release mode using the command line:

```bash
# Create a build directory
mkdir -p build
cd build
# Configure CMake, i.e. for Release builds for consumer Pascal and newer GPUs
cmake .. -DCMAKE_CUDA_ARCHITECTURES=61 -DCMAKE_BUILD_TYPE=Release
# Build all targets using 8 threads.
cmake --build . --target all -j 8
# Or directly via make
make all -j 8
```

Or, if building under windows you must specify the Visual Studio version to use.

```cmd
cmake .. -A x64 -G "Visual Studio 16 2019"
```

When using multi-config generators such as visual studio, the build configuration is selected at build time rather than at CMake configure time.

```cmd
cmake --build . --config Release --target ALL_BUILD -j 8
```

Alternatively, open the generated `.sln` file manually, or open visual studio via CMake:

```cmd
cmake --open .
```

### Lint Only Configuration

The project can be configured to allow linting without the need for CUDA or OpenGL to be available (i.e. CI).

To do this, set the `FLAMEGPU_ALLOW_LINT_ONLY` CMake option to `ON`. I.e.:

```bash
cmake .. -DFLAMEGPU_ALLOW_LINT_ONLY=ON
cmake --build . --target lint_flamegpu_visualiser
```

## Git Tags

To allow simple version pinning of the visualiser from within the main [FLAMEGPU/FLAMEGPU2](https://github.com/FLAMEGPU/FLAMEGPU2) repository, tags are used to identify which version of the visualisation repository are used in each release of the main repository.

The main repository uses semantic versioning, with tags `vX.Y.Z[-PRERELEASE]`.
Versions of this repository included by the main repository are tagged with `flamegpu-X.Y.Z[-PRERELEASE]` to allow standalone versioning options in the future.

I.e. the initial alpha release of FLAMEGPU/FLAMEGPU2 `v2.0.0-alpha` corresponds to the `flamegpu-2.0.0-alpha` tag. Subsequent pre-releases may not update the tag to be an exact match.
