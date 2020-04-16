# Parity with main FGPU2 CMake
cmake_minimum_required(VERSION VERSION 3.12 FATAL_ERROR)

# Don't create installation scripts (and hide CMAKE_INSTALL_PREFIX from cmake-gui)
set(CMAKE_SKIP_INSTALL_RULES TRUE)
set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE INTERNAL ""  FORCE)

find_package(Git)
if (NOT GIT_FOUND)
  message(FATAL_ERROR "git is required for building")
endif ()

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
    endif ()
    # Run CMake build (this only downloads, it is built at build time)
    execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sdl2-download
    )
    if (result)
        message(WARNING
                "Download step for sdl2 failed: ${result}\n"
                "Attempting to continue\n")
    endif ()
ENDMACRO()
# Pull in and build glew
MACRO (download_glew)
    configure_file(cmake/glew/CMakeLists.txt.in glew-download/CMakeLists.txt)
    # Run CMake generate
    execute_process(
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/glew-download
        )
    if (result)
        message(WARNING
                "CMake step for glew failed: ${result}\n")
    endif ()
    # Run CMake build (this only downloads, it is built at build time)
    execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/glew-download
    )
    if (result)
        message(WARNING
                "Download step for glew failed: ${result}\n"
                "Attempting to continue\n")
    endif ()
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
    endif ()
    # Run CMake build (this only downloads, it is built at build time)
    execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/glm-download
    )
    if (result)
        message(WARNING
                "Download step for glm failed: ${result}\n"
                "Attempting to continue\n")
    endif ()
ENDMACRO()
# Pull in freetype
MACRO (download_freetype)
    configure_file(cmake/freetype/CMakeLists.txt.in freetype-download/CMakeLists.txt)
    # Run CMake generate
    execute_process(
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/freetype-download
        )
    if (result)
        message(WARNING 
                "CMake step for freetype failed: ${result}\n")
    endif()
    # Run CMake build (this only downloads, it is built at build time)
    execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/freetype-download
    )
    if (result)
        message(WARNING 
                "Download step for freetype failed: ${result}\n"
                "Attempting to continue\n"
                "You may want to disable BUILD_TESTS")
    endif()
ENDMACRO()

project(flamegpu2_visualiser CUDA CXX)

include(FindOpenGL)
if (NOT TARGET OpenGL::GL)
    message(FATAL_ERROR "OpenGL is required for building")
endif ()
download_glm()
if (UNIX)
    # Linux users need to install it via their package manager
    # e.g. sudo apt install cmake libsdl2-dev g++
    find_package(SDL2)
    if (NOT SDL2_FOUND)
      message(FATAL_ERROR "sdl2 is required for building, install it via your package manager.\n"
                          "e.g. sudo apt install libsdl2-dev")
    endif ()    
    find_package(GLEW)
    if (NOT GLEW_FOUND)
      message(FATAL_ERROR "glew is required for building, install it via your package manager.\n"
                          "e.g. sudo apt install libglew-dev")
    endif ()
    find_package(Freetype)
    if (NOT FREETYPE_FOUND)
      message(FATAL_ERROR "freetype is required for building, install it via your package manager.\n"
                          "e.g. sudo apt install libfreetype-dev")
    endif ()
    if (NOT TARGET OpenGL::GLU)
      message(FATAL_ERROR "GLU is required for building, install it via your package manager.\n"
                          "e.g. sudo apt install libglu1-mesa-dev")
    endif ()
    find_package(Fontconfig)
    if (NOT Fontconfig_FOUND)
        message(FATAL_ERROR "Fontconfig is required for visualisation, install it via your package manager.\n"
                          "e.g. sudo apt install libfontconfig1-dev")
    endif()
else()
    if (NOT TARGET OpenGL::GLU)
      message(FATAL_ERROR "GLU is required for building")
    endif ()
    #sdl
    download_sdl2()
    set(SDL2_DIR ${CMAKE_CURRENT_BINARY_DIR}/sdl2)
    configure_file(cmake/sdl2/sdl2-config.cmake sdl2/sdl2-config.cmake)
    find_package(SDL2 REQUIRED)
    #glew
    download_glew()
    set(GLEW_DIR ${CMAKE_CURRENT_BINARY_DIR}/glew)
    configure_file(cmake/glew/glew-config.cmake glew/glew-config.cmake)
    find_package(GLEW REQUIRED)
    #freetype    
    download_freetype()
    # Force disable zlib/libpng, to avoid linker errors if theyre pseudo found in system
    set(CMAKE_DISABLE_FIND_PACKAGE_ZLIB ON CACHE BOOL "" FORCE)
    set(CMAKE_DISABLE_FIND_PACKAGE_LIBPNG ON CACHE BOOL "" FORCE)
    set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
    mark_as_advanced(FORCE CMAKE_DISABLE_FIND_PACKAGE_ZLIB)
    mark_as_advanced(FORCE CMAKE_DISABLE_FIND_PACKAGE_LIBPNG)
    mark_as_advanced(FORCE SKIP_INSTALL_ALL)
    mark_as_advanced(FORCE FT_WITH_BZIP2)
    mark_as_advanced(FORCE FT_WITH_HARFBUZZ)
    mark_as_advanced(FORCE FT_WITH_PNG)
    mark_as_advanced(FORCE FT_WITH_ZLIB)
    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/freetype-src
                     ${CMAKE_CURRENT_BINARY_DIR}/freetype-build
                     EXCLUDE_FROM_ALL
                     )
endif ()


# GCC requires -lpthreads for std::thread
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)


# Prepare list of include files
SET(VISUALISER_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/FLAMEGPU_Visualisation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/config/AgentStateConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/config/ModelConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/config/Stock.h
    # Items from sdl_exp
)
# Prepare list of source files
SET(VISUALISER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Visualiser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/VisException.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/Resources.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FLAMEGPU_Visualisation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Visualiser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/AgentStateConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/ModelConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/VisException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/Resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/warnings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/fonts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/fonts.cpp
    # .h from sdl_exp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/camera/NoClipCamera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interface/Camera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interface/FBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interface/Reloadable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interface/Renderable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interface/RenderTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interface/Viewport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/Material.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/multipass/BackBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/multipass/FrameBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/multipass/FrameBufferAttachment.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/multipass/RenderBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/multipass/RenderPass.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/buffer/BufferCore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/buffer/ShaderStorageBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/buffer/UniformBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/lights/DirectionalLight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/lights/DirectionalLight.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/lights/LightsBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/lights/PointLight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/lights/PointLight.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/lights/SpotLight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/lights/SpotLight.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/ShaderCore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/ShaderHeader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/Shaders.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/ShadersVec.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/Texture.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/Texture2D.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/Texture2D_Multisample.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/TextureBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/TextureCubeMap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/cuda.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/GLcheck.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/StringUtils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Axis.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Draw.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Entity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HUD.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Overlay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Sprite2D.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Text.h
    # .cpp from sdl_exp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/camera/NoClipCamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model/Material.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/multipass/BackBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/multipass/FrameBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/multipass/RenderBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/multipass/RenderPass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/buffer/BufferCore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/buffer/ShaderStorageBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/buffer/UniformBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/lights/LightsBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/ShaderCore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader/Shaders.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/Texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/Texture2D.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/Texture2D_Multisample.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/TextureBuffer.cu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/texture/TextureCubeMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util/cuda.cu
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Axis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HUD.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Overlay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Sprite2D.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Text.cpp
)
SET(VISUALISER_ALL
    ${VISUALISER_INCLUDE}
    ${VISUALISER_SRC}
)


# Use C++14 standard - std::make_unique is 14 not 11
# Specify using C++14 standard
if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED true)
endif ()
if (NOT UNIX)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif ()
# Define output
add_library("${PROJECT_NAME}" STATIC ${VISUALISER_ALL})
# Set up include dirs
target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/glm-src)
target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${GLEW_INCLUDE_DIRS})
if (FREETYPE_FOUND) # Only use this if we aren't building it ourselves
    target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${FREETYPE_INCLUDE_DIRS})
endif ()
if(Fontconfig_FOUND)
    target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${Fontconfig_INCLUDE_DIRS})
endif()
target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/external")
target_include_directories("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Add the targets we depend on (this does link and include)
# This propagates to any project that uses this as a dependency
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_link_libraries("${PROJECT_NAME}" "legacy_stdio_definitions") # This won't be required if we rebuild freetype with newer than vs2013
endif()

# If gcc, need to add flag for std::experimental::filesystem
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries("${PROJECT_NAME}" "-lstdc++fs")
endif()


target_link_libraries("${PROJECT_NAME}" "${SDL2_LIBRARIES}")
target_link_libraries("${PROJECT_NAME}" "${GLEW_LIBRARIES}")
target_link_libraries("${PROJECT_NAME}" freetype)
target_link_libraries("${PROJECT_NAME}" OpenGL::GL)
target_link_libraries("${PROJECT_NAME}" OpenGL::GLU)
target_link_libraries("${PROJECT_NAME}" Threads::Threads)
if(Fontconfig_FOUND)
    target_link_libraries("${PROJECT_NAME}" Fontconfig::Fontconfig)
endif()
if(WIN32)
target_link_libraries("${PROJECT_NAME}" "dwrite.lib")
endif()


# Enable parallel compilation
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
endif ()

# Resources
# NOTE: The build step for resources embeds them as base64, this should not be used for large files.
include(cmake/CMakeRC/CMakeRC.cmake)
SET(RESOURCES_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/instanced_default.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/material_flat.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/material_phong.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/default.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/text.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/icosphere.obj
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/sphere.obj
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/cube.obj
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/teapot.obj
)
cmrc_add_resource_library(resources ${RESOURCES_ALL})
message(STATUS "${PROJECT_NAME}")
target_link_libraries("${PROJECT_NAME}" resources)

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

# Stuff borrowed from FGPU2/common.cmake
# Option to group CMake generated projects into folders in supported IDEs
option(CMAKE_USE_FOLDERS "Enable folder grouping of projects in IDEs." ON)
mark_as_advanced(CMAKE_USE_FOLDERS)
# Define a function to add a lint target.
find_file(CPPLINT NAMES cpplint cpplint.exe)
if (CPPLINT)
    # Add custom target for linting this
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Output in visual studio format
        add_custom_target(
            "lint_${PROJECT_NAME}"
            COMMAND ${CPPLINT} "--output" "vs7"
            ${VISUALISER_ALL}
        )
    else()
        # Output in default format
        add_custom_target(
            "lint_${PROJECT_NAME}"
            COMMAND ${CPPLINT}
            ${VISUALISER_ALL}
        )
    endif ()
    # Don't trigger this target on ALL_BUILD or Visual Studio 'Rebuild Solution'
    set_target_properties("lint_${PROJECT_NAME}" PROPERTIES EXCLUDE_FROM_ALL TRUE)
    # set_target_properties("lint_${NAME}" PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE #This breaks all_lint in visual studio
    # Add the custom target as a dependency of the global lint target
    if (TARGET all_lint)
        add_dependencies(all_lint lint_${PROJECT_NAME})
    endif ()
    # Put within Lint filter
    if (CMAKE_USE_FOLDERS)
      set_property(GLOBAL PROPERTY USE_FOLDERS ON)
      set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "FLAMEGPU")
      set_property(TARGET "lint_${PROJECT_NAME}" PROPERTY FOLDER "Lint")
      if (TARGET freetype)
        set_property(TARGET "freetype" PROPERTY FOLDER "FLAMEGPU/Dependencies")
        set_property(TARGET "resources" PROPERTY FOLDER "FLAMEGPU/Dependencies")
      endif ()
    endif ()
endif ()

