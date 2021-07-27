# Glu is required, but must be installed externally.
if(UNIX)
    if (NOT TARGET OpenGL::GLU)
        message(FATAL_ERROR "GLU is required for building, install it via your package manager.\n"
                        "e.g. sudo apt install libglu1-mesa-dev")
    endif ()
elseif(WIN32)
    if (NOT TARGET OpenGL::GLU)
        message(FATAL_ERROR "GLU is required for building")
    endif ()
endif()