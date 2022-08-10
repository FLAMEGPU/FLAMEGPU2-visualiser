# SDL2 
# Ensure that SDL2 is available, either by finding it, or downloading it if required (on windows)
if(UNIX)
    # On Linux, if SDL2 is not available the user is instructed to install it themselves.
    find_package(SDL2)
    if (NOT SDL2_FOUND)
        message(FATAL_ERROR "sdl2 is required for building, install it via your package manager.\n"
                        "e.g. sudo apt install libsdl2-dev")
    endif ()
elseif(WIN32)
    # On windows, always download manually. There are issues with find_package and multi-config generators where a release library will be found, but no debug library, which can break things.
    # Declare source properties
    # As the URL method is used for download, set the policy if available
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    FetchContent_Declare(
        SDL2
        URL "https://www.libsdl.org/release/SDL2-devel-2.0.12-VC.zip"
    )
    FetchContent_GetProperties(SDL2)
    if(NOT SDL2_POPULATED)
        # Download content
        FetchContent_Populate(SDL2)
        # Create a Cmake configuraiton file for SDL2 (the download is not cmake aware)
        set(SDL2_DIR ${sdl2_SOURCE_DIR})
        configure_file(${CMAKE_CURRENT_LIST_DIR}/sdl2-config.cmake.in ${sdl2_SOURCE_DIR}/sdl2-config.cmake @ONLY)
        # Find it again. Erroring if it cannot be found.
        find_package(SDL2 REQUIRED)
    endif()
endif()