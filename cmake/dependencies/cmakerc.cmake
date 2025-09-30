###########
# CMakeRC #
###########

include(FetchContent)
# As the URL method is used for download, set the policy if available
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
# Temporary CMake >= 3.30 fix https://github.com/FLAMEGPU/FLAMEGPU2/issues/1223
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/modules/ ${CMAKE_MODULE_PATH})
include(FetchContent)

FetchContent_Declare(
    cmakerc
    GIT_REPOSITORY    "https://github.com/vector-of-bool/cmrc.git"
    GIT_TAG           952ffddba731fc110bd50409e8d2b8a06abbd237 # latest commit with cmake 3.27+ support
)
FetchContent_GetProperties(cmakerc)
if(NOT cmakerc_POPULATED)
    FetchContent_Populate(cmakerc)
endif()

# include the CMakeRC source
include(${cmakerc_SOURCE_DIR}/CMakeRC.cmake)

# Mark some CACHE vars advanced for a cleaner GUI
mark_as_advanced(FETCHCONTENT_QUIET)
mark_as_advanced(FETCHCONTENT_BASE_DIR)
mark_as_advanced(FETCHCONTENT_FULLY_DISCONNECTED)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED)
mark_as_advanced(FETCHCONTENT_SOURCE_DIR_CMAKERC)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED_CMAKERC)