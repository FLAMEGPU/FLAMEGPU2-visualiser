# SDL2 
# Ensure that SDL2 is available, either by finding it, or downloading it if required (on windows)
if(UNIX OR FLAMEGPU_BUILD_PYTHON_CONDA)
    # On Linux, if SDL2 is not available the user is instructed to install it themselves.
    find_package(SDL2)
    if (NOT SDL2_FOUND)
        message(FATAL_ERROR "sdl2 is required for building, install it via your package manager.\n"
                        "e.g. sudo apt install libsdl2-dev")
    else()
        # If no imported targets are provided, make one, using variables from the older find_package(SDL2).
        # Tested on ubuntu 20.04 / libsdl2-dev 2.0.10+dfsg1-3
        if (NOT TARGET SDL2::SDL2)
            # If we have a ${libdir} from the above find_package sdl2, use that.
            if(${libdir})
                set(SDL2_LIBDIR ${libdir})
            endif()
            add_library(SDL2::SDL2 SHARED IMPORTED)
            set_target_properties(SDL2::SDL2 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${SDL2_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}SDL2${CMAKE_SHARED_LIBRARY_SUFFIX}")
            endif()
    endif()
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
        # Find SDL2, only looking in the generated directory in config mode
        find_package(SDL2 REQUIRED CONFIG 
            PATHS ${sdl2_SOURCE_DIR}
            NO_CMAKE_PATH
            NO_CMAKE_ENVIRONMENT_PATH
            NO_SYSTEM_ENVIRONMENT_PATH
            NO_CMAKE_PACKAGE_REGISTRY
            NO_CMAKE_SYSTEM_PATH)
    endif()
endif()

mark_as_advanced(FETCHCONTENT_QUIET)
mark_as_advanced(FETCHCONTENT_BASE_DIR)
mark_as_advanced(FETCHCONTENT_FULLY_DISCONNECTED)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED) 
mark_as_advanced(FETCHCONTENT_SOURCE_DIR_SDL2)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED_SDL2)
mark_as_advanced(SDL2_DIR)
