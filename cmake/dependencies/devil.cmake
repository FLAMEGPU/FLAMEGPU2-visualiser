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
    include(FetchContent)
    # Temporary CMake >= 3.30 fix https://github.com/FLAMEGPU/FLAMEGPU2/issues/1223
    if(POLICY CMP0169)
        cmake_policy(SET CMP0169 OLD)
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
        # Find DevIL, only looking in the generated directory in config mode
        find_package(DevIL REQUIRED CONFIG 
            PATHS ${devil_SOURCE_DIR}
            NO_CMAKE_PATH
            NO_CMAKE_ENVIRONMENT_PATH
            NO_SYSTEM_ENVIRONMENT_PATH
            NO_CMAKE_PACKAGE_REGISTRY
            NO_CMAKE_SYSTEM_PATH)
    endif()
endif()

# Mark some CACHE vars advanced for a cleaner GUI
mark_as_advanced(FETCHCONTENT_QUIET)
mark_as_advanced(FETCHCONTENT_BASE_DIR)
mark_as_advanced(FETCHCONTENT_FULLY_DISCONNECTED)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED) 
mark_as_advanced(FETCHCONTENT_SOURCE_DIR_DEVIL)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED_DEVIL)
mark_as_advanced(IL_INCLUDE_DIR)
mark_as_advanced(IL_LIBRARIES)
mark_as_advanced(ILU_LIBRARIES)
mark_as_advanced(ILUT_LIBRARIES)