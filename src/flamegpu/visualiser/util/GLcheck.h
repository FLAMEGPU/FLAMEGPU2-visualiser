#ifndef SRC_FLAMEGPU_VISUALISER_UTIL_GLCHECK_H_
#define SRC_FLAMEGPU_VISUALISER_UTIL_GLCHECK_H_

#include <GL/glew.h>

#include <cstdio>
#include <cstdlib>
#include "VisException.h"

namespace flamegpu {
namespace visualiser {

// Provide a method to convert a GLnum to an error string, as OpenGL does not provide this, and it's not worth pulling in GLU for.
// Error messages based on https://www.khronos.org/opengl/wiki/OpenGL_Error
inline static const char * glErrorString(GLenum code) {
    switch (code) {
        case GL_INVALID_ENUM: return "Invalid Enumerator (GL_INVALID_ENUM)";
        case GL_INVALID_VALUE: return "Invalid Value (GL_INVALID_VALUE)";
        case GL_INVALID_OPERATION: return "Invalid Operation (GL_INVALID_OPERATION)";
        case GL_STACK_OVERFLOW: return "Stack Overflow (GL_STACK_OVERFLOW)";
        case GL_STACK_UNDERFLOW: return "Stack Underflow (GL_STACK_UNDERFLOW)";
        case GL_OUT_OF_MEMORY: return "Out of Memory (GL_OUT_OF_MEMORY)";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "Operating on invlida OpenGL Frame Buffer(GL_INVALID_FRAMEBUFFER_OPERATION)";
        // with OpenGL 4.5 or ARB_KHR_robustness:
        case GL_CONTEXT_LOST: return "OpenGL context lost due to GPU reset (GL_CONTEXT_LOST)";
        // Reprecated in OpenGL 3.0, removed in OpenGL 3.1:
        case GL_TABLE_TOO_LARGE: return "Removed/Deprecated Error Code (GL_TABLE_TOO_LARGE)";
        // Value indicating no more errors.
        case GL_NO_ERROR: return "No more OpenGL errors (GL_NO_ERROR)";
        default: return "Unknown OpenGL Error";
    }
}

#ifdef _DEBUG  // VS standard debug flag

inline static void HandleGLError(const char *file, int line) {
    GLuint error = glGetError();
    if (error != GL_NO_ERROR) {
        throw GLError("%s(%i) GL Error Occurred;\n%s\n", file, line, reinterpret_cast<const char *>(glErrorString(error)));
    }
}

#define GL_CALL(err) err; HandleGLError(__FILE__, __LINE__)
#define GL_CHECK() (HandleGLError(__FILE__, __LINE__))

#else  // ifdef _DEBUG
// Remove the checks when running release mode.
#define GL_CALL(err) err
#define GL_CHECK()

#endif  // ifdef  _DEBUG

inline static void InitGlew() {
    // https:// www.opengl.org/wiki/OpenGL_Loading_Library#GLEW_.28OpenGL_Extension_Wrangler.29
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        THROW GLError("Glew Init failed;\n%s\n", reinterpret_cast<const char *>(glewGetErrorString(err)));
    }
    glGetError();  // This error can be ignored, GL_INVALID_ENUMERANT
}
#define GLEW_INIT() (InitGlew())

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_UTIL_GLCHECK_H_
