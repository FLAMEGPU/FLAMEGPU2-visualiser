#include "shader/buffer/BufferCore.h"

#include <memory>

#include "util/GLcheck.h"

BufferCore::BufferCore(GLenum bufferType, GLint bindPoint, size_t size, void* data)
    : size(size)
    , bufferName(0)
    , bufferBindPoint(bindPoint)
    , bufferType(bufferType) {
    if (this->size > static_cast<size_t>(maxSize(bufferType))) {
        THROW VisAssert("BufferCore::BufferCore(): Size (%u) exceeds max buffer size (%u)!\n", this->size, maxSize(bufferType));
    }
    GL_CALL(glGenBuffers(1, &bufferName));
    GL_CALL(glBindBuffer(bufferType, bufferName));
    GL_CALL(glBindBufferBase(bufferType, bufferBindPoint, bufferName));
    GL_CALL(glBufferData(bufferType, size, data, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(bufferType, 0));
}
BufferCore::~BufferCore() {
    GL_CALL(glDeleteBuffers(1, &bufferName));
}
void BufferCore::setData(void *data, size_t _size) {
    this->size = _size == 0 ? this->size : _size;
    if (this->size > static_cast<size_t>(maxSize(bufferType))) {
        THROW VisAssert("BufferCore::setData(): Size (%u) exceeds max buffer size (%u)!\n", this->size, maxSize(bufferType));
    }
    GL_CALL(glBindBuffer(bufferType, bufferName));
    GL_CALL(glBufferData(bufferType, this->size, data, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(bufferType, 0));
}
void BufferCore::setData(void *data, size_t _size, size_t offset) {
    visassert(_size + offset <= this->size);
    if (_size + offset > static_cast<size_t>(maxSize(bufferType))) {
        THROW VisAssert("BufferCore::setData(): _size (%u) + offset (%u) exceeds max buffer size (%u)!\n", _size, offset, maxSize(bufferType));
    }
    GL_CALL(glBindBuffer(bufferType, bufferName));
    GL_CALL(glBufferSubData(bufferType, offset, _size, reinterpret_cast<void*>(data)));
    GL_CALL(glBindBuffer(bufferType, 0));
}
void BufferCore::getData(void *dataReturn, size_t _size, size_t offset) {
    _size = _size == 0 ? this->size : _size;
    GL_CALL(glBindBuffer(bufferType, bufferName));
    GL_CALL(glGetBufferSubData(bufferType, offset, _size, reinterpret_cast<void*>(dataReturn)));
    GL_CALL(glBindBuffer(bufferType, 0));
}
void const *BufferCore::mapBufferRead() {
    return mapBuffer(GL_READ_ONLY);
}
void *BufferCore::mapBufferWrite() {
    return mapBuffer(GL_WRITE_ONLY);
}
void *BufferCore::mapBufferReadWrite() {
    return mapBuffer(GL_READ_WRITE);
}
void *BufferCore::mapBuffer(GLenum access) {
    GL_CALL(glBindBuffer(bufferType, bufferName));
    GL_CALL(glMapBuffer(bufferType, access));
    GL_CALL(glBindBuffer(bufferType, 0));
    void *rtn;
    GL_CALL(glGetBufferPointerv(bufferType, GL_BUFFER_MAP_POINTER, &rtn));
    return rtn;
}
void BufferCore::unmapBuffer() {
    GL_CALL(glBindBuffer(bufferType, bufferName));
    GL_CALL(glUnmapBuffer(bufferType));
    GL_CALL(glBindBuffer(bufferType, 0));
}
GLenum BufferCore::getBlockType() {
    if (bufferType == GL_UNIFORM_BUFFER)
        return GL_UNIFORM_BLOCK;
    if (bufferType == GL_SHADER_STORAGE_BUFFER)
        return GL_SHADER_STORAGE_BLOCK;
    if (bufferType == GL_TRANSFORM_FEEDBACK_BUFFER)
        return GL_TRANSFORM_FEEDBACK_BUFFER;
    // if (bufferType == GL_ATOMIC_COUNTER_BUFFER)
    fprintf(stderr, "Buffer type was unexpected.\n");
    return GL_INVALID_ENUM;
}
GLint BufferCore::maxSize(GLenum bufferType) {
    if (bufferType == GL_UNIFORM_BUFFER) {
        static GLint maxUniformBufferSize = 0;
        if (!maxUniformBufferSize)
            GL_CALL(glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBufferSize));  // 65536@1080gtx (65KB)
        return maxUniformBufferSize;
    } else if (bufferType == GL_SHADER_STORAGE_BUFFER) {
        static GLint maxSSBBufferSize = 0;
        if (!maxSSBBufferSize)
            GL_CALL(glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &maxSSBBufferSize));  // 2147483647@1080gtx (2147MB)
        return maxSSBBufferSize;
    } else if (bufferType == GL_TRANSFORM_FEEDBACK_BUFFER) {
        return 0;
    } else if (bufferType == GL_ATOMIC_COUNTER_BUFFER) {
        static GLint maxACBufferSize = 0;
        if (maxACBufferSize <= 0) {
            GL_CALL(glGetIntegerv(GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE, &maxACBufferSize));  // 65536@1080gtx (65KB)
        }
        return maxACBufferSize;
    }
    return 0;
}
GLint BufferCore::maxBuffers(GLenum bufferType) {
    if (bufferType == GL_UNIFORM_BUFFER) {
        static GLint maxUniformBindings = -1;
        if (maxUniformBindings <= 0) {
            GL_CALL(glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUniformBindings));  // 84@1080gtx
        }
        return maxUniformBindings;
    } else if (bufferType == GL_SHADER_STORAGE_BUFFER) {
        static GLint maxSSBBindings = -1;
        if (maxSSBBindings <= 0) {
            GL_CALL(glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maxSSBBindings));  // 96@1080gtx
        }
        return maxSSBBindings;
    } else if (bufferType == GL_TRANSFORM_FEEDBACK_BUFFER) {
        static GLint maxTFBBindings = -1;
        if (maxTFBBindings <= 0) {
            GL_CALL(glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_BUFFERS, &maxTFBBindings));  // 4@1080gtx
            // These are per shader invocation, they work differently to regular bindings
        }
        return maxTFBBindings;
    } else if (bufferType == GL_ATOMIC_COUNTER_BUFFER) {
        static GLint maxACBindings = -1;
        if (maxACBindings <= 0) {
            GL_CALL(glGetIntegerv(GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS, &maxACBindings));  // 8@1080gtx
        }
        return maxACBindings;
    }
    return 0;
}

// Comment out this include if not making use of Shaders/ShaderCore
#include "shader/ShaderCore.h"
#ifdef SRC_SHADER_SHADERCORE_H_
bool ShaderCore::addBuffer(const char *bufferNameInShader, const std::shared_ptr<BufferCore> &buffer) {  // Treat it similar to texture binding points
    return addBuffer(bufferNameInShader, buffer->getType(), buffer->getBufferBindPoint());
}
#endif
