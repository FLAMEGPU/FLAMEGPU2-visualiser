# Minimum CMake version 3.25.2 for CUDA --std=c++20
cmake_minimum_required(VERSION 3.25.2...4.1.1 FATAL_ERROR)

project(FLAMEGPU2_VISUALISER LANGUAGES NONE)

# Don't create installation scripts (and hide CMAKE_INSTALL_PREFIX from cmake-gui)
set(CMAKE_SKIP_INSTALL_RULES TRUE)
set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE INTERNAL ""  FORCE)

# Ensure this is not an in-source build
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/OutOfSourceOnly.cmake)

# Define some CMake Options which apply to all (sub) projects

# Option to promote compilation warnings to error, useful for strict CI
option(FLAMEGPU_WARNINGS_AS_ERRORS "Promote compilation warnings to errors" OFF)

# Option to group CMake generated projects into folders in supported IDEs
option(CMAKE_USE_FOLDERS "Enable folder grouping of projects in IDEs." ON)
mark_as_advanced(CMAKE_USE_FOLDERS)

# Add a new cmake option, to allow lint_only configurations.
option(FLAMEGPU_ALLOW_LINT_ONLY "Allow the project to be configured for lint-only builds" OFF)
mark_as_advanced(FLAMEGPU_ALLOW_LINT_ONLY)

# Add the src subdirectory cmake project.
add_subdirectory(src "${PROJECT_BINARY_DIR}/FLAMEGPU_visualiser")
