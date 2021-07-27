# Freetype
# Ensure that freetype is available, either by finding freetype, or dowloadin it if required (on windows). 

if(UNIX)
    find_package(Freetype)
    if (NOT FREETYPE_FOUND)
        message(FATAL_ERROR "freetype is required for building, install it via your package manager.\n"
                            "e.g. sudo apt install libfreetype-dev")
    endif ()
elseif(WIN32)
    # On windows, if Freetype cannot be found, it is downloaded and CMake is told how to find it (via config mode)
    find_package(Freetype QUIET)
    if(NOT Freetype_FOUND)
        # Declare source properties
        # Alt official mirror: https://git.sv.nongnu.org/r/freetype/freetype2.git
        FetchContent_Declare(
            freetype
            GIT_REPOSITORY    https://git.savannah.gnu.org/git/freetype/freetype2.git
            GIT_TAG           tags/VER-2-10-1
        )

        # Use variables to control freetype CMake options:
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

        # Freetype includes CMakeLists.txt so we can use MakeAvailable
        FetchContent_MakeAvailable(freetype)
        
        # Freetype with MSVC on windows emits warnings at /W1 (default). Use a cmake function defined elsewhere to suppress all warnings on the freetype target
        include(${CMAKE_CURRENT_LIST_DIR}/../warnings.cmake)
        if(TARGET freetype)
            DisableCompilerWarnings(TARGET freetype)
        endif()
    endif()
endif()