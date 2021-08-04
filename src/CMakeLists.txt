# Parity with main FGPU2 CMake
cmake_minimum_required(VERSION VERSION 3.15 FATAL_ERROR)

# Find git which is a hard dependency
find_package(Git)
if (NOT GIT_FOUND)
  message(FATAL_ERROR "git is required for building")
endif ()

# Set the project name, but do not specify languages immediately so we can have lint only builds.
project(flamegpu_visualiser)

# handle cpplint.
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/cpplint.cmake)

# Check to see if CUDA is available.
include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
    enable_language(C)
    enable_language(CXX)
    enable_language(CUDA)
    # CUDA_ARCH is used to control the compute capabilities to build for, via cmake/cuda_arch.cmake.
    # @todo - use the same value as the parent project if imported.
    include(${CMAKE_CURRENT_LIST_DIR}/../cmake/cuda_arch.cmake)
endif()

# Check to see if OpenGL is available.
include(FindOpenGL)

# If CUDA was not found, or opengl was not found we cannot build, but can lint, otherwise tehre should be a fatal error
set(LINT_ONLY_BUILD "OFF")
if(ALLOW_LINT_ONLY AND CPPLINT AND (NOT OPENGL_FOUND OR NOT CMAKE_CUDA_COMPILER))
    set(LINT_ONLY_BUILD "ON")
endif()

# Define the list of source files early, for lint-only configurations
# Prepare list of include files
SET(VISUALISER_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/flamegpu/visualiser/FLAMEGPU_Visualisation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/flamegpu/visualiser/config/AgentStateConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/flamegpu/visualiser/config/ModelConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/flamegpu/visualiser/config/LineConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/flamegpu/visualiser/config/Stock.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/flamegpu/visualiser/config/TexBufferConfig.h
)
# Prepare list of source files
SET(VISUALISER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/Visualiser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/util/VisException.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/util/Resources.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/FLAMEGPU_Visualisation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/Visualiser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/config/AgentStateConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/config/ModelConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/util/VisException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/util/Resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/util/warnings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/util/fonts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/util/fonts.cpp
    # .h from sdl_exp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/camera/NoClipCamera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/interface/Camera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/interface/FBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/interface/Reloadable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/interface/Renderable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/interface/RenderTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/interface/Viewport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/model/Material.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/multipass/BackBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/multipass/FrameBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/multipass/FrameBufferAttachment.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/multipass/RenderBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/multipass/RenderPass.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/buffer/BufferCore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/buffer/ShaderStorageBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/buffer/UniformBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/lights/DirectionalLight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/lights/DirectionalLight.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/lights/LightsBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/lights/PointLight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/lights/PointLight.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/lights/SpotLight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/lights/SpotLight.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/ShaderCore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/ShaderHeader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/Shaders.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/ShadersVec.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/PositionFunction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/DirectionFunction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/ScaleFunction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/texture/Texture.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/texture/Texture2D.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/texture/Texture2D_Multisample.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/texture/TextureBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/texture/TextureCubeMap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/ui/Overlay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/ui/SplashScreen.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/ui/Sprite2D.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/ui/Text.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/util/cuda.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/util/GLcheck.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/util/StringUtils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/Axis.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/Draw.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/Entity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/HUD.h
    # .cpp from sdl_exp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/camera/NoClipCamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/model/Material.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/multipass/BackBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/multipass/FrameBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/multipass/RenderBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/multipass/RenderPass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/buffer/BufferCore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/buffer/ShaderStorageBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/buffer/UniformBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/lights/LightsBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/ShaderCore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/Shaders.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/PositionFunction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/DirectionFunction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/shader/ScaleFunction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/texture/Texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/texture/Texture2D.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/texture/Texture2D_Multisample.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/texture/TextureBuffer.cu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/texture/TextureCubeMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/ui/Overlay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/ui/SplashScreen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/ui/Sprite2D.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/ui/Text.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/util/cuda.cu
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/Axis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/Draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/Entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flamegpu/visualiser/HUD.cpp
)
SET(VISUALISER_ALL
    ${VISUALISER_INCLUDE}
    ${VISUALISER_SRC}
)

# Add the lint target.
if(CPPLINT)
    new_linter_target("${PROJECT_NAME}" "${VISUALISER_ALL}")
endif()

if(LINT_ONLY_BUILD)
    # All should include all_lint if lint only build.
    set_target_properties(all_lint PROPERTIES EXCLUDE_FROM_ALL FALSE)
    if (NOT CPPLINT)
        message(FATAL_ERROR "Cpplint is required for lint-only builds.")
    endif()
    message(STATUS "Lint-only build configuraiton due to missing dependencies")
    if (NOT OPENGL_FOUND)
        message(STATUS "OpenGL is required for building")
    endif ()
    if (NOT CMAKE_CUDA_COMPILER)
        message(STATUS "CUDA is required for building")
    endif ()
    # return from the file.
    return()
endif()

# If not a lint only build, fatal error on missing dependencies.
if (NOT OPENGL_FOUND)
    message(FATAL_ERROR "OpenGL is required for building")
endif ()
if (NOT CMAKE_CUDA_COMPILER)
    message(FATAL_ERROR "CUDA is required for building")
endif ()

# Include the warnings cmake file. Disabling all warning by default, and providing a mechanism to opt-in for some targets
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/warnings.cmake)

# Find third party dependencies. Downloading them if required. 
# glm
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/dependencies/glm.cmake)
# glu (not downloaded)
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/dependencies/glu.cmake)
# glew
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/dependencies/glew.cmake)
# sdl
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/dependencies/sdl2.cmake)
# fontconfig (linux)    
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/dependencies/fontconfig.cmake)
# freetype    
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/dependencies/freetype.cmake)
# DevIL
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/dependencies/devil.cmake)

# GCC requires -lpthreads for std::thread
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# On windows, define NOMINMAX to remove the annoying defined min and max macros
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_definitions(NOMINMAX)
endif()

# Select the C++ standard to use 
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/cxxstd.cmake)

# Define output
add_library("${PROJECT_NAME}" STATIC ${VISUALISER_ALL})

# Set warning flags for the target.
EnableCompilerWarnings(TARGET "${PROJECT_NAME}")

# Prevent fscanf warnings for msvc. 
if (WIN32)
  target_compile_definitions("${PROJECT_NAME}" PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
endif()

# Ensure VS uses SYSTEM if available.
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.10)
        # These flags don't currently have any effect on how CMake passes system-private includes to msvc (VS 2017+)
        set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "/external:I")
        set(CMAKE_INCLUDE_SYSTEM_FLAG_CUDA "/external:I")
        # VS 2017+
        target_compile_options("${PROJECT_NAME}" PRIVATE "$<$<COMPILE_LANGUAGE:C,CXX>:/experimental:external>")
    endif()
endif()

# Set up include dirs
target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${glm_INCLUDE_DIRS})
target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${GLEW_INCLUDE_DIRS})
if (FREETYPE_FOUND) # Only use this if we aren't building it ourselves
    target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${FREETYPE_INCLUDE_DIRS})
endif ()
target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${IL_INCLUDE_DIR})
if(Fontconfig_FOUND)
    target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${Fontconfig_INCLUDE_DIRS})
endif()
target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../external")
target_include_directories("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_include_directories("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/")

# Enable fPIC for linux shared library linking
set_property(TARGET "${PROJECT_NAME}" PROPERTY POSITION_INDEPENDENT_CODE ON)

# Add the targets we depend on (this does link and include)
# This propagates to any project that uses this as a dependency
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_link_libraries("${PROJECT_NAME}" "legacy_stdio_definitions") # This won't be required if we rebuild freetype with newer than vs2013
endif()

# If gcc, need to add flag for std::experimental::filesystem (for c++14 support)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries("${PROJECT_NAME}" "-lstdc++fs")
endif()

# Strip trailiing whitespace from SDL2_Libraries for Ubuntu1604 / libsdl2-dev 2.0.4
string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
target_link_libraries("${PROJECT_NAME}" "${SDL2_LIBRARIES}")
target_link_libraries("${PROJECT_NAME}" "${GLEW_LIBRARIES}")
target_link_libraries("${PROJECT_NAME}" freetype)
target_link_libraries("${PROJECT_NAME}" "${IL_LIBRARIES}")
target_link_libraries("${PROJECT_NAME}" OpenGL::GL)
target_link_libraries("${PROJECT_NAME}" OpenGL::GLU)
target_link_libraries("${PROJECT_NAME}" Threads::Threads)
if(TARGET Fontconfig::Fontconfig)
    target_link_libraries("${PROJECT_NAME}" Fontconfig::Fontconfig)
endif()
if(WIN32)
    target_link_libraries("${PROJECT_NAME}" "dwrite.lib")
endif()

# Enable parallel compilation on MSVC
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options("${PROJECT_NAME}" PRIVATE "$<$<COMPILE_LANGUAGE:C,CXX>:/MP>")
endif ()

# Resources
# NOTE: The build step for resources embeds them as base64, this should not be used for large files.
include(${CMAKE_CURRENT_LIST_DIR}/../cmake/CMakeRC/CMakeRC.cmake)
SET(RESOURCES_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/flamegpu_icon.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/pyflamegpu_icon.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/flamegpu.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/pyflamegpu.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/material_flat.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/material_phong.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/default.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/text.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/sprite2d.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/color.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/color_noshade.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/icosphere.obj
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/sphere.obj
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/cube.obj
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/teapot.obj
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/stuntplane.obj
    # Tdir: These shaders are setup to receive an appended rotation function
    # Tcolor: These shaders are setup to receive an appended color function
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/instanced_default_Tpos_Tdir_Tscale.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/instanced_default_Tcolor_Tpos_Tdir_Tscale.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/material_flat_Tcolor.frag
)
cmrc_add_resource_library(resources ${RESOURCES_ALL} WHENCE ${CMAKE_CURRENT_SOURCE_DIR}/..)
# Enable fPIC for resources (for wheel packaging)
set_property(TARGET "resources" PROPERTY POSITION_INDEPENDENT_CODE ON)

# Link resources and the visualiser static lib
target_link_libraries("${PROJECT_NAME}" resources)

if (CMAKE_USE_FOLDERS)
    # Setup Visual Studio (and eclipse) filters
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    #src/.h
    set(T_SRC "${VISUALISER_SRC}")
    list(FILTER T_SRC INCLUDE REGEX ".*\.(h|hpp|cuh)$")
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src/headers FILES ${T_SRC})
    #src/.cpp
    set(T_SRC "${VISUALISER_SRC}")
    list(FILTER T_SRC EXCLUDE REGEX ".*\.(h|hpp|cuh)$")
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX src/src FILES ${T_SRC})
    #include/*
    set(T_SRC "${VISUALISER_INCLUDE}")
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../include PREFIX include FILES ${T_SRC})

    # Place the visualiser target in the FLAMEGPU folder (for consistency)
    if (TARGET "${PROJECT_NAME}")
        set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "FLAMEGPU")
    endif()
    # The all_lint target blongs in the Lint folder.
    if (TARGET all_lint)
        set_property(TARGET all_lint PROPERTY FOLDER "Lint")
    endif()
    # Place dependency targets in a folder.
    if (TARGET freetype)
        set_property(TARGET "freetype" PROPERTY FOLDER "FLAMEGPU/Dependencies")
    endif()
    if (TARGET resources)
        set_property(TARGET "resources" PROPERTY FOLDER "FLAMEGPU/Dependencies")
    endif()
endif()
