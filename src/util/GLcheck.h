#ifndef SRC_UTIL_GLCHECK_H_
#define SRC_UTIL_GLCHECK_H_

#include <GL/glew.h>

#include <cstdio>
#include <cstdlib>


// Define EXIT_ON_ERROR to cause the program to exit when a GL error occurs
// Define FORCE_OPTIMUS to force dedicated GPU usage in laptops

#ifdef _DEBUG  // VS standard debug flag

inline static void HandleGLError(const char *file, int line) {
    GLuint error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("%s(%i) GL Error Occurred;\n%s\n", file, line, reinterpret_cast<const char *>(gluErrorString(error)));
#if EXIT_ON_ERROR
        getchar();
        exit(1);
#endif
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
        fprintf(stderr, "Error: %s\n", reinterpret_cast<const char *>(glewGetErrorString(err)));
        getchar();
        exit(1);
    }
    glGetError();  // This error can be ignored, GL_INVALID_ENUMERANT
}
#define GLEW_INIT() (InitGlew())

#endif  // SRC_UTIL_GLCHECK_H_
