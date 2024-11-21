#include "flamegpu/visualiser/shader/buffer/ShaderStorageBuffer.h"

#include <set>

#include "flamegpu/visualiser/util/GLcheck.h"

namespace flamegpu {
namespace visualiser {

std::set<GLint> ShaderStorageBuffer::allocatedBindPoints;
ShaderStorageBuffer::ShaderStorageBuffer(size_t size, void* data)
    : BufferCore(GL_SHADER_STORAGE_BUFFER, allocateBindPoint(), size, data) { }
ShaderStorageBuffer::~ShaderStorageBuffer() {
    allocatedBindPoints.erase(bufferBindPoint);
}

GLint ShaderStorageBuffer::allocateBindPoint() {
    if (allocatedBindPoints.size() == static_cast<size_t>(MaxBuffers())) {
        THROW VisAssert("Shader Storage Buffer Bindings exceeded!\nLimit = %d\n\nsdl_exp ShaderStorageBuffer objs are not designed for sharing buffer bindings.", MaxBuffers());
    }
    for (unsigned int i = MaxBuffers() - 1; i >= 0; --i) {
        if (allocatedBindPoints.find(i) == allocatedBindPoints.end()) {
            allocatedBindPoints.insert(i);
            return i;
        }
    }
    THROW VisAssert("ShaderStorageBuffer::allocateBindPoint(): This point should never be reached!");
}

GLint ShaderStorageBuffer::MaxSize() {
    return BufferCore::maxSize(GL_SHADER_STORAGE_BUFFER);
}
GLint ShaderStorageBuffer::MaxBuffers() {
    return BufferCore::maxBuffers(GL_SHADER_STORAGE_BUFFER);
}

}  // namespace visualiser
}  // namespace flamegpu
