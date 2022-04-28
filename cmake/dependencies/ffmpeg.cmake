##########
# ffmpeg #
##########
if(UNIX)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../modules/ ${CMAKE_MODULE_PATH})
    find_package(FFMPEG COMPONENTS avcodec avfilter)
    if (NOT (FFMPEG_INCLUDE_DIRS AND FFMPEG_avcodec_FOUND AND FFMPEG_avfilter_FOUND))
        message(FATAL_ERROR "FFMPEG is required for building, install it via your package manager.\n"
                            "e.g. sudo apt install ffmpeg libavcodex-dev libavfilter-dev")
    endif()
elseif(WIN32)
    message(FATAL_ERROR "FFMPEG support is only enabled for Linux builds.")
endif()
if(NOT FFMPEG_FOUND)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../modules/ ${CMAKE_MODULE_PATH})
    include(FetchContent)

    # Head of master at point BugFix for NVRTC support was merged
    FetchContent_Declare(
        glm
        URL "https://github.com/g-truc/glm/archive/66062497b104ca7c297321bd0e970869b1e6ece5.zip"
    )
    FetchContent_GetProperties(glm)
    if(NOT glm_POPULATED)
        FetchContent_Populate(glm)
        # glm CMake wants to generate the find file in a system location, so handle it manually
        set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${glm_SOURCE_DIR}")
        find_package(glm REQUIRED)
        # Include path is ${glm_INCLUDE_DIRS}
    endif()
endif()