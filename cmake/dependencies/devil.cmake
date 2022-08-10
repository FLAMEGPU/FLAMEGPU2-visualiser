# DevIL
# Ensure that DevIL is available, either by finding DevIL, or by downloading DevIL if required (on windows) 
if(UNIX)
    # On Linux, if DevIL is not available the user is instructed to install it themselves.
    find_package(DevIL)
    if (NOT DevIL_FOUND)
        message(FATAL_ERROR "DevIL is required for image loading, install it via your package manager.\n"
                        "e.g. sudo apt install libdevil-dev")
    endif()
elseif(WIN32)

    # As the URL method is used for download, set the policy if available
    if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
    endif()

    # On windows, always download manually. There are issues with find_package and multi-config generators where a release library will be found, but no debug library, which can break things.
    # Declare source properties
    FetchContent_Declare(
        devil
        URL "http://downloads.sourceforge.net/openil/DevIL-Windows-SDK-1.8.0.zip"
    )
    FetchContent_GetProperties(devil)
    if(NOT devil_POPULATED)
        # Download content
        FetchContent_Populate(devil)
        # Create a Cmake configuraiton file for devil (the download is not cmake aware)
        set(DevIL_DIR ${devil_SOURCE_DIR})
        configure_file(${CMAKE_CURRENT_LIST_DIR}/devil-config.cmake.in ${devil_SOURCE_DIR}/devil-config.cmake @ONLY)
        # Find it again. Erroring if it cannot be found.
        find_package(DevIL REQUIRED NO_MODULE)
    endif()
endif()