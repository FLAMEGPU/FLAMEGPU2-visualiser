# Parity with main FGPU2 CMake
cmake_minimum_required(VERSION VERSION 3.15 FATAL_ERROR)

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
# Pull in and build DevIL
MACRO (download_devil)
    configure_file(cmake/devil/CMakeLists.txt.in devil-download/CMakeLists.txt)
    # Run CMake generate
    execute_process(
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/devil-download
        )
    if (result)
        message(WARNING
                "CMake step for DevIL failed: ${result}\n")
    endif ()
    # Run CMake build (this only downloads, it is built at build time)
    execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/devil-download
    )
    if (result)
        message(WARNING
                "Download step for DevIL failed: ${result}\n"
                "Attempting to continue\n")
    endif ()
ENDMACRO()

# Set the project name, but do not specify languages immediately so we can have lint only builds.
project(flamegpu2_visualiser)

# Option to group CMake generated projects into folders in supported IDEs
option(CMAKE_USE_FOLDERS "Enable folder grouping of projects in IDEs." ON)
mark_as_advanced(CMAKE_USE_FOLDERS)


# Add a new cmake option, to allow lint_only configurations.
option(ALLOW_LINT_ONLY "Allow the project to be configured for lint-only builds" OFF)
mark_as_advanced(ALLOW_LINT_ONLY)

find_file(CPPLINT NAMES cpplint cpplint.exe)
if(CPPLINT)

    if(NOT TARGET all_lint)
        add_custom_target(all_lint)
        set_target_properties(all_lint PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()
    # Define a function to add a lint target.
    function(new_linter_target NAME SRC)
        # Don't lint external files
        list(FILTER SRC EXCLUDE REGEX "^${FLAMEGPU_ROOT}/externals/.*")

        LIST(APPEND CPPLINT_ARGS "")
            
        # Specify output format for msvc highlighting
        if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            LIST(APPEND CPPLINT_ARGS "--output" "vs7")
        endif()
        # Set the --root argument if included as a sub project.
        if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
            file(RELATIVE_PATH LINT_ROOT ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
            LIST(APPEND CPPLINT_ARGS "--root=${LINT_ROOT}")
        endif()


        # Add the lint_ target
        add_custom_target(
            "lint_${PROJECT_NAME}"
            COMMAND ${CPPLINT} ${CPPLINT_ARGS}
            ${SRC}
        )

        # Don't trigger this target on ALL_BUILD or Visual Studio 'Rebuild Solution'
        set_target_properties("lint_${NAME}" PROPERTIES EXCLUDE_FROM_ALL TRUE)
        # set_target_properties("lint_${NAME}" PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE #This breaks all_lint in visual studio
        # Add the custom target as a dependency of the global lint target
        if(TARGET all_lint)
            add_dependencies(all_lint lint_${NAME})
        endif()
        # Put within Lint filter
        # Put within Lint filter
        if (CMAKE_USE_FOLDERS)
            set_property(GLOBAL PROPERTY USE_FOLDERS ON)
            # set_property(TARGET "${PROJECT_NAME}" PROPERTY FOLDER "FLAMEGPU")
            set_property(TARGET "lint_${PROJECT_NAME}" PROPERTY FOLDER "Lint")
        endif ()
    endfunction()
else()
    # Don't create this message multiple times
    message( 
        " cpplint: NOT FOUND!\n"
        " Lint projects will not be generated.\n"
        " Please install cpplint as described on https://pypi.python.org/pypi/cpplint.\n"
        " In most cases command 'pip install --user cpplint' should be sufficient.")
    function(new_linter_target NAME SRC)
    endfunction()
endif()


# Check to see if CUDA is available.
include(CheckLanguage)
check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
    enable_language(C)
    enable_language(CXX)
    enable_language(CUDA)
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
    ${CMAKE_CURRENT_SOURCE_DIR}/include/flamegpu/visualiser/FLAMEGPU_Visualisation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/flamegpu/visualiser/config/AgentStateConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/flamegpu/visualiser/config/ModelConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/flamegpu/visualiser/config/LineConfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/flamegpu/visualiser/config/Stock.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/flamegpu/visualiser/config/TexBufferConfig.h
    # Items from sdl_exp
)
# Prepare list of source files
SET(VISUALISER_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/Visualiser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/util/VisException.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/util/Resources.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/FLAMEGPU_Visualisation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/Visualiser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/config/AgentStateConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/config/ModelConfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/util/VisException.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/util/Resources.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/util/warnings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/util/fonts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/util/fonts.cpp
    # .h from sdl_exp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/camera/NoClipCamera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/interface/Camera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/interface/FBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/interface/Reloadable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/interface/Renderable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/interface/RenderTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/interface/Viewport.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/model/Material.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/multipass/BackBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/multipass/FrameBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/multipass/FrameBufferAttachment.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/multipass/RenderBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/multipass/RenderPass.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/buffer/BufferCore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/buffer/ShaderStorageBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/buffer/UniformBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/lights/DirectionalLight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/lights/DirectionalLight.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/lights/LightsBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/lights/PointLight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/lights/PointLight.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/lights/SpotLight.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/lights/SpotLight.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/ShaderCore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/ShaderHeader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/Shaders.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/ShadersVec.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/DirectionFunction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/ScaleFunction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/texture/Texture.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/texture/Texture2D.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/texture/Texture2D_Multisample.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/texture/TextureBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/texture/TextureCubeMap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/ui/Overlay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/ui/SplashScreen.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/ui/Sprite2D.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/ui/Text.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/util/cuda.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/util/GLcheck.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/util/StringUtils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/Axis.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/Draw.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/Entity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/HUD.h
    # .cpp from sdl_exp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/camera/NoClipCamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/model/Material.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/multipass/BackBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/multipass/FrameBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/multipass/RenderBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/multipass/RenderPass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/buffer/BufferCore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/buffer/ShaderStorageBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/buffer/UniformBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/lights/LightsBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/ShaderCore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/Shaders.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/DirectionFunction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/shader/ScaleFunction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/texture/Texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/texture/Texture2D.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/texture/Texture2D_Multisample.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/texture/TextureBuffer.cu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/texture/TextureCubeMap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/ui/Overlay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/ui/SplashScreen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/ui/Sprite2D.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/ui/Text.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/util/cuda.cu
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/Axis.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/Draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/Entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flamegpu/visualiser/HUD.cpp
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
else()
    if (NOT OPENGL_FOUND)
        message(FATAL_ERROR "OpenGL is required for building")
    endif ()
    if (NOT CMAKE_CUDA_COMPILER)
        message(FATAL_ERROR "CUDA is required for building")
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
        find_package(DevIL)
        if (NOT DevIL_FOUND)
        message(FATAL_ERROR "DevIL is required for image loading, install it via your package manager.\n"
                            "e.g. sudo apt install libdevil-dev")
        endif()
        if (NOT TARGET OpenGL::GLU)
        message(FATAL_ERROR "GLU is required for building, install it via your package manager.\n"
                            "e.g. sudo apt install libglu1-mesa-dev")
        endif ()
        find_package(Fontconfig)
        if (NOT Fontconfig_FOUND)
        message(FATAL_ERROR "Fontconfig is required for visualisation, install it via your package manager.\n"
                        "e.g. sudo apt install libfontconfig1-dev")
        endif()
    else ()
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
        set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz ON CACHE BOOL "" FORCE)
        set(FT_WITH_HARFBUZZ OFF CACHE BOOL "" FORCE)
        set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
        mark_as_advanced(FORCE CMAKE_DISABLE_FIND_PACKAGE_ZLIB)
        mark_as_advanced(FORCE CMAKE_DISABLE_FIND_PACKAGE_LIBPNG)
        mark_as_advanced(FORCE CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz)
        mark_as_advanced(FORCE SKIP_INSTALL_ALL)
        mark_as_advanced(FORCE FT_WITH_BZIP2)
        mark_as_advanced(FORCE FT_WITH_HARFBUZZ)
        mark_as_advanced(FORCE FT_WITH_PNG)
        mark_as_advanced(FORCE FT_WITH_ZLIB)
        add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/freetype-src
                        ${CMAKE_CURRENT_BINARY_DIR}/freetype-build
                        EXCLUDE_FROM_ALL
                        )
        #DevIL
        download_devil()
        set(DEVIL_DIR ${CMAKE_CURRENT_BINARY_DIR}/devil)
        configure_file(cmake/devil/devil-config.cmake devil/devil-config.cmake)
        find_package(DEVIL REQUIRED NO_MODULE)
    endif ()


    # GCC requires -lpthreads for std::thread
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)

    
    # Select the CXX standard to use. 
    if(NOT FLAMEGPU_CXX_STD)
        # FLAME GPU is c++14, however due to MSVC 16.10 regressions we build as 17 if possible, else 14. 
        # 14 Support is still required (CUDA 10.x, swig?).
        # Start by assuming both should be availble.
        set(CXX17_SUPPORTED ON)
        # CMake 3.18 adds CUDA CXX 17, 20
        # CMake 3.10 adds CUDA CXX 14
        if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
            # 17 OK
        elseif(CMAKE_VERSION VERSION_GREATER_EQUAL 3.10)
            set(CXX17_SUPPORTED OFF)
        else()
            message(FATAL_ERROR "CMAKE ${CMAKE_VERSION} does not support -std=c++14")
        endif()
        # CUDA 11.0 adds CXX 17
        # CUDA 9.0 adds CXX 14
        if(CMAKE_CUDA_COMPILER_VERSION VERSION_GREATER_EQUAL 11.0.0)
            # 17 is ok.
        elseif(CMAKE_CUDA_COMPILER_VERSION VERSION_GREATER_EQUAL 9.0.0)
            # 14 is ok, 17 is not.
            set(CXX17_SUPPORTED OFF)
        else()
            # Fatal Error.
            message(FATAL_ERROR "CUDA ${CMAKE_CUDA_COMPILER_VERSION} does not support -std=c++14")
        endif()

        # VS 2019 16.10.0 breaks CXX 14 + cuda. - 1930? 19.29.30037?
        # VS 2017 version 15.3 added /std:c++17 - 1911
        # MSVC VS2015 Update 3 added /std:c++14 >= 1900 && < 1910? 
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.29)
                # 17 required.
                if(NOT CXX17_SUPPORTED)
                    message(FATAL_ERROR "MSVC >= 19.29 requires CMake >= 3.18 and CUDA >= 11.0")
                endif()
            elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.11)
                # 17 available?
            elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.10)
                set(CXX17_SUPPORTED OFF)
            else()
                message(FATAL_ERROR "MSVC ${CMAKE_CXX_COMPILER_VERSION} does not support -std=c++14")
            endif()
        endif()

        # gcc supported C++17 since 5, so any version supported by cuda 10+ (no need to check, a configure time error will already occur.)
        # Inside source code, __STDC_VERSION__ can be used on msvc, which will have a value such as 201710L for c++17

        # Set a cmake variable so this is only calcualted once, and can be applied afterwards.
        if(CXX17_SUPPORTED)
            set(FLAMEGPU_CXX_STD 17)
        else()
            set(FLAMEGPU_CXX_STD 14)
        endif()
        # if(NOT CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
        #     # Forward to the parent scope so it persists between calls.
        #     set(FLAMEGPU_CXX_STD ${FLAMEGPU_CXX_STD} PARENT_SCOPE)
        # endif()
    endif()

    # @future - set this on a per target basis using set_target_properties?
    set(CMAKE_CXX_EXTENSIONS OFF)
    if(NOT DEFINED CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD ${FLAMEGPU_CXX_STD})
        set(CMAKE_CXX_STANDARD_REQUIRED true)
    endif()
    if(NOT DEFINED CMAKE_CUDA_STANDARD)
        set(CMAKE_CUDA_STANDARD ${FLAMEGPU_CXX_STD})
        set(CMAKE_CUDA_STANDARD_REQUIRED True)
    endif()


    # Define output
    add_library("${PROJECT_NAME}" STATIC ${VISUALISER_ALL})
    # Set up include dirs
    target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/glm-src)
    target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})
    target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${GLEW_INCLUDE_DIRS})
    if (FREETYPE_FOUND) # Only use this if we aren't building it ourselves
        target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${FREETYPE_INCLUDE_DIRS})
    endif ()
    target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${IL_INCLUDE_DIR})
    if(Fontconfig_FOUND)
        target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE ${Fontconfig_INCLUDE_DIRS})
    endif()
    target_include_directories("${PROJECT_NAME}" SYSTEM PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/external")
    target_include_directories("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
    target_include_directories("${PROJECT_NAME}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

    # Enable fPIC for linux shared library linking
    set_property(TARGET "${PROJECT_NAME}" PROPERTY POSITION_INDEPENDENT_CODE ON)

    # Add the targets we depend on (this does link and include)
    # This propagates to any project that uses this as a dependency
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_link_libraries("${PROJECT_NAME}" "legacy_stdio_definitions") # This won't be required if we rebuild freetype with newer than vs2013
    endif()

    # If gcc, need to add flag for std::experimental::filesystem
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
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/flamegpu_icon.png
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/pyflamegpu_icon.png
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/flamegpu.png
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/pyflamegpu.png
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/material_flat.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/material_phong.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/default.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/text.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/sprite2d.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/color.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/color_noshade.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/icosphere.obj
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/sphere.obj
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/cube.obj
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/teapot.obj
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/stuntplane.obj
# Tdir: These shaders are setup to receive an appended rotation function
# Tcolor: These shaders are setup to receive an appended color function
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/instanced_default_Tdir_Tscale.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/instanced_default_Tcolor_Tdir_Tscale.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/material_flat_Tcolor.frag
    )
    cmrc_add_resource_library(resources ${RESOURCES_ALL})
    # Enable fPIC for resources (for wheel packaging)
    set_property(TARGET "resources" PROPERTY POSITION_INDEPENDENT_CODE ON)

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

    # Put freetype in a folder.
    if (CMAKE_USE_FOLDERS)
        set_property(GLOBAL PROPERTY USE_FOLDERS ON)
        if (TARGET freetype)
            set_property(TARGET "freetype" PROPERTY FOLDER "FLAMEGPU/Dependencies")
            set_property(TARGET "resources" PROPERTY FOLDER "FLAMEGPU/Dependencies")
        endif()
    endif()
endif()