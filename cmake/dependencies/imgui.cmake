#########
# imgui #
#########

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/modules/ ${CMAKE_MODULE_PATH})
include(FetchContent)
cmake_policy(SET CMP0079 NEW)
# As the URL method is used for download, set the policy if available
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
# Temporary CMake >= 3.30 fix https://github.com/FLAMEGPU/FLAMEGPU2/issues/1223
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()
# Change the source_dir to allow inclusion via imgui/imgui.h rather than imgui.h
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.88
    GIT_SHALLOW    1
    SOURCE_DIR     ${FETCHCONTENT_BASE_DIR}/imgui-src/imgui
    GIT_PROGRESS   ON
    # UPDATE_DISCONNECTED   ON
)

# imgui does not include anything

# @todo - try finding the package first, assuming it sets system correctly when used.
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    
    # The imgui repository does not include a CMakeLists.txt (or similar)
    # Create our own cmake target for imgui.

    # If the target does not already exist, add it.
    # @todo - make this far more robust.
    if(NOT TARGET imgui)
        # make a dynamically generated CMakeLists.txt which can be add_subdirectory'd instead, so that the .vcxproj goes in a folder. Just adding a project doesn't work.        
        configure_file(${CMAKE_CURRENT_LIST_DIR}/imgui-CMakeLists.txt.in ${FETCHCONTENT_BASE_DIR}/imgui-src/CMakeLists.txt @ONLY)        
        # We've now created CMakeLists.txt so we can use MakeAvailable
        add_subdirectory(${imgui_SOURCE_DIR}/.. ${imgui_BINARY_DIR})
    endif()
endif()

mark_as_advanced(FETCHCONTENT_QUIET)
mark_as_advanced(FETCHCONTENT_BASE_DIR)
mark_as_advanced(FETCHCONTENT_FULLY_DISCONNECTED)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED) 
mark_as_advanced(FETCHCONTENT_SOURCE_DIR_IMGUI)
mark_as_advanced(FETCHCONTENT_UPDATES_DISCONNECTED_IMGUI)