# GLEW
# Ensure that GLEW is available, either by finding GLEW, or by downloading GLEW if required (on windows) 

if(UNIX)
    # On Linux, if glew is not available the user is instructed to install it themselves.
    find_package(GLEW)
    if (NOT GLEW_FOUND)
        message(FATAL_ERROR "glew is required for building, install it via your package manager.\n"
                            "e.g. sudo apt install libglew-dev")
    endif ()
elseif(WIN32)
    # On windows, if GLEW cannot be found, it is downloaded and CMake is told how to find it (via config mode)
    find_package(GLEW QUIET)
    if(NOT GLEW_FOUND)
        # Declare source properties
        FetchContent_Declare(
            glew
            URL "https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip"
        )
        FetchContent_GetProperties(glew)
        if(NOT glew_POPULATED)
            # Download content
            FetchContent_Populate(glew)
            # Create a Cmake configuraiton file for glew (the download is not cmake aware)
            set(GLEW_DIR ${glew_SOURCE_DIR})
            configure_file(${CMAKE_CURRENT_LIST_DIR}/glew-config.cmake.in ${glew_SOURCE_DIR}/glew-config.cmake @ONLY)
            # Find it again. Erroring if it cannot be found.
            find_package(GLEW REQUIRED)
        endif()
    endif()
endif()