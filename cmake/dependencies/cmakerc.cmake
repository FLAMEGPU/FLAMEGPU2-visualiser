###########
# CMakeRC #
###########

include(FetchContent)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/modules/ ${CMAKE_MODULE_PATH})
include(FetchContent)

# Specify an invalid SOURCE_SUBDIR to prevent FetchContent_MakeAvailable from adding the CMakeLists.txt
FetchContent_Declare(
    cmakerc
    GIT_REPOSITORY    "https://github.com/vector-of-bool/cmrc.git"
    GIT_TAG           952ffddba731fc110bd50409e8d2b8a06abbd237 # latest commit with cmake 3.27+ support
    SOURCE_SUBDIR "do_not_use_add_subirectory"
)
# Download cmakerc, but do not add_subdirectory or find a package or create a target
FetchContent_MakeAvailable(cmakerc)

# include the CMakeRC source
include(${cmakerc_SOURCE_DIR}/CMakeRC.cmake)

# Mark some CACHE vars advanced for a cleaner GUI
mark_as_advanced(FETCHCONTENT_QUIET)
mark_as_advanced(FETCHCONTENT_BASE_DIR)
mark_as_advanced(FETCHCONTENT_FULLY_DISCONNECTED)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED)
mark_as_advanced(FETCHCONTENT_SOURCE_DIR_CMAKERC)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED_CMAKERC)