#ifndef SRC_SHADER_BUFFER_UNIFORMBUFFER_H_
#define SRC_SHADER_BUFFER_UNIFORMBUFFER_H_
#include <set>

#include "util/GLcheck.h"
#include "shader/buffer/BufferCore.h"

/**
 * Representative of GL_UNIFORM_BUFFER
 * These can be used for reading from during shader execution
 * Particularly useful for passing light/material info or arrays of transformation matrices
 * The spec requires a max size of atleast 16KB (likely much bigger, e.g. 65KB on modern NV hardware)
 */
class UniformBuffer : public BufferCore {
 public:
    explicit UniformBuffer(size_t bytes, void* data = nullptr);
    ~UniformBuffer();
    static GLint MaxSize();
    static GLint MaxBuffers();

 private:
    GLint allocateBindPoint();
    static std::set<GLint> allocatedBindPoints;
};

#endif  // SRC_SHADER_BUFFER_UNIFORMBUFFER_H_
