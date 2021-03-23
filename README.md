## Introduction

This repository contains the components for the FLAMEGPU2 visualiser. This provides the capability for FLAMEGPU2 models to display a real-time visualisation. This repository is automatically pulled in by [FLAMEGPU2](https://github.com/FLAMEGPU/FLAMEGPU2_dev) when the `VISUALISATION` option is enabled within the CMake configuration. It is unlikely to be useful independently.



### Lint Only Configuration

The project can be configured to allow linting without the need for CUDA or OpenGL to be available (i.e. CI). 

To do this, set the `ALLOW_LINT_ONLY` CMake option to `ON`.