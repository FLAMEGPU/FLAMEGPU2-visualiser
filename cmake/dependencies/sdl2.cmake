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
    # On windows, if SDL2 cannot be found, it is downloaded and CMake is told how to find it (via config mode)
    find_package(SDL2 QUIET)
    if(NOT SDL2_FOUND)
        # Declare source properties
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
endif()