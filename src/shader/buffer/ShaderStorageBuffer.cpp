#include "shader/buffer/ShaderStorageBuffer.h"

#include <cassert>

#include "util/GLcheck.h"

std::set<GLint> ShaderStorageBuffer::allocatedBindPoints;
ShaderStorageBuffer::ShaderStorageBuffer(size_t size, void* data)
    : BufferCore(GL_SHADER_STORAGE_BUFFER, allocateBindPoint(), size, data) { }
ShaderStorageBuffer::~ShaderStorageBuffer() {
    allocatedBindPoints.erase(bufferBindPoint);
}

GLint ShaderStorageBuffer::allocateBindPoint() {
    if (allocatedBindPoints.size() == size_t(MaxBuffers())) {
        // TODO: Better exception implementation.
        // char buff[1024];
        // snprintf(buff, sizeof(buff), "Shader Storage Buffer Bindings exceeded!\nLimit = %d\n\nsdl_exp ShaderStorageBuffer objs are not designed for sharing buffer bindings.", MaxBuffers());
        throw std::exception();

        //// @TODO - upgrade exceptions.
    }
    for (unsigned int i = MaxBuffers() - 1; i >= 0; --i) {
        if (allocatedBindPoints.find(i) == allocatedBindPoints.end()) {
            allocatedBindPoints.insert(i);
            return i;
        }
    }
    assert(false);  // Should never reach here
    return -1;
}

GLint ShaderStorageBuffer::MaxSize() {
    return BufferCore::maxSize(GL_SHADER_STORAGE_BUFFER);
}
GLint ShaderStorageBuffer::MaxBuffers() {
    return BufferCore::maxBuffers(GL_SHADER_STORAGE_BUFFER);
}
