# Parity with main FGPU2 CMake
cmake_minimum_required(VERSION VERSION 3.12 FATAL_ERROR)

# Don't create installation scripts (and hide CMAKE_INSTALL_PREFIX from cmake-gui)
set(CMAKE_SKIP_INSTALL_RULES TRUE)
set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE INTERNAL ""  FORCE)

find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "git is required for building")
endif()

# Pull in and build SDL2
MACRO (download_sdl2)
    configure_file(cmake/sdl2/CMakeLists.txt.in sdl2-download/CMakeLists.txt)
    # Run CMake generate
    execute_process(
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sdl2-download
        )
    if (result)
        message(WARNING 
                "CMake step for sdl2 failed: ${result}\n")
    endif()
    # Run CMake build (this only downloads, it is built at build time)
    execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sdl2-download
    )
    if (result)
        message(WARNING 
                "Download step for sdl2-build failed: ${result}\n"
                "Attempting to continue\n")
    endif()
ENDMACRO()
# Pull in GLM
MACRO (download_glm)
    configure_file(cmake/glm/CMakeLists.txt.in glm-download/CMakeLists.txt)
    # Run CMake generate
    execute_process(
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/glm-download
        )
    if (result)
        message(WARNING 
                "CMake step for glm failed: ${result}\n")
    endif()
    # Run CMake build (this only downloads, it is built at build time)
    execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/glm-download
    )
    if (result)
        message(WARNING 
                "Download step for glm-build failed: ${result}\n"
                "Attempting to continue\n")
    endif()
ENDMACRO()

download_glm()
if(UNIX)
    # Linux users need to install it via their package manager
    # e.g. sudo apt install cmake libsdl2-dev g++
    find_package(SDL2)

    if(NOT SDL2_FOUND)
      message(FATAL_ERROR "sdl2 is required for building, install it via your package manager.\n"
                          "e.g. sudo apt install libsdl2-dev")
    endif()
else()
    download_sdl2()
    set(SDL2_DIR ${CMAKE_CURRENT_BINARY_DIR}/sdl2)
    configure_file(cmake/sdl2/sdl2-config.cmake sdl2/sdl2-config.cmake)
    find_package(SDL2 REQUIRED)
endif()

project(FLAMEGPU2_Visualiser LANGUAGES CXX)

# Prepare list of include files
SET(VISUALISER_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/test.h
)
# Prepare list of source files
SET(VISUALISER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)
SET(VISUALISER_ALL
    ${VISUALISER_INCLUDE}
    ${VISUALISER_SRC}
)

# Define output
add_library("${PROJECT_NAME}" STATIC ${VISUALISER_ALL})

# Use C++14 standard - std::make_unique is 14 not 11
# Specify using C++14 standard
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED true)
endif()


# Set up include dirs
target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/glm-src)
target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/sdl2/include)
target_include_directories("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Add the targets we depend on (this does link and include)
# This propagates to any project that uses this as a dependency
target_link_libraries("${PROJECT_NAME}" "${SDL2_LIBRARIES}")

# Enable parallel compilation
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
endif()


# Setup Visual Studio (and eclipse) filters
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
#src/.h
set(T_SRC "${VISUALISER_ALL}")
list(FILTER T_SRC INCLUDE REGEX "^${CMAKE_CURRENT_SOURCE_DIR}/src")
list(FILTER T_SRC INCLUDE REGEX ".*\.(h|hpp|cuh)$")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX headers FILES ${T_SRC})
#src/.cpp
set(T_SRC "${VISUALISER_ALL}")
list(FILTER T_SRC INCLUDE REGEX "^${CMAKE_CURRENT_SOURCE_DIR}/src")
list(FILTER T_SRC EXCLUDE REGEX ".*\.(h|hpp|cuh)$")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${T_SRC})
#./.h
set(T_SRC "${VISUALISER_ALL}")
list(FILTER T_SRC EXCLUDE REGEX "^${CMAKE_CURRENT_SOURCE_DIR}/src")
list(FILTER T_SRC INCLUDE REGEX ".*\.(h|hpp|cuh)$")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX headers FILES ${T_SRC})
#./.cpp
set(T_SRC "${VISUALISER_ALL}")
list(FILTER T_SRC EXCLUDE REGEX "^${CMAKE_CURRENT_SOURCE_DIR}/src")
list(FILTER T_SRC EXCLUDE REGEX ".*\.(h|hpp|cuh)$")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX src FILES ${T_SRC})