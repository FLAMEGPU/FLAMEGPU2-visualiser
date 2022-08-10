# GLEW
# Ensure that GLEW is available, either by finding GLEW, or by downloading GLEW if required (on windows) 

if(UNIX)
    # On Linux, if glew is not available the user is instructed to install it themselves.
    # Users can opt-into static glew, by setting -DGLEW_USE_STATIC_LIBS=ON
    find_package(GLEW)
    if (NOT GLEW_FOUND)
        message(FATAL_ERROR "glew is required for building, install it via your package manager.\n"
                            "e.g. sudo apt install libglew-dev")
    endif ()
elseif(WIN32)
    # As the URL method is used for download, set the policy if available
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    # On windows, always download manually. There are issues with find_package and multi-config generators where a release library will be found, but no debug library, which can break things.
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