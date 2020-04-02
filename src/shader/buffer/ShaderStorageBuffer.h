#ifndef SRC_SHADER_BUFFER_SHADERSTORAGEBUFFER_H_
#define SRC_SHADER_BUFFER_SHADERSTORAGEBUFFER_H_
#include <GL/glew.h>

#include <set>

#include "shader/buffer/BufferCore.h"
/**
 * Representative of GL_SHADER_STORAGE_BUFFER
 * These can be used for reading from and writing to during shader execution
 * Particularly useful for compute shaders
 * The spec requires a max size of atleast 128mb
 * Access is however expected to be slower than uniforms, due to the in shader writeable nature
 */
class ShaderStorageBuffer : public BufferCore {
 public:
    explicit ShaderStorageBuffer(size_t bytes, void* data = nullptr);
    ~ShaderStorageBuffer();
    static GLint MaxSize();
    static GLint MaxBuffers();

 private:
    GLint allocateBindPoint();
    static std::set<GLint> allocatedBindPoints;
};

#endif  // SRC_SHADER_BUFFER_SHADERSTORAGEBUFFER_H_
