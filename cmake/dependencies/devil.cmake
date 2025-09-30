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
    # On windows, always download manually. There are issues with find_package and multi-config generators where a release library will be found, but no debug library, which can break things.
    # Declare source properties
    # Specify an invalid SOURCE_SUBDIR to prevent FetchContent_MakeAvailable from adding the CMakeLists.txt
    FetchContent_Declare(
        devil
        URL "http://downloads.sourceforge.net/openil/DevIL-Windows-SDK-1.8.0.zip"
        SOURCE_SUBDIR "do_not_use_add_subirectory"
    )
    FetchContent_GetProperties(devil)
    # Download content
    FetchContent_MakeAvailable(devil)
    # Create a Cmake configuraiton file for devil (the download is not cmake aware)
    set(DevIL_DIR ${devil_SOURCE_DIR})
    if (NOT EXISTS "${devil_SOURCE_DIR}/devil-config.cmake")
        configure_file(${CMAKE_CURRENT_LIST_DIR}/devil-config.cmake.in ${devil_SOURCE_DIR}/devil-config.cmake @ONLY)
    endif()
    # Find DevIL, only looking in the generated directory in config mode
    find_package(DevIL REQUIRED CONFIG 
        PATHS ${devil_SOURCE_DIR}
        NO_CMAKE_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_PACKAGE_REGISTRY
        NO_CMAKE_SYSTEM_PATH)
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