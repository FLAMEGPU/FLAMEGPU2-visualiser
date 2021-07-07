#include "flamegpu/visualiser/shader/buffer/UniformBuffer.h"

#include <memory>

#include "flamegpu/visualiser/util/GLcheck.h"

namespace flamegpu {
namespace visualiser {

std::set<GLint> UniformBuffer::allocatedBindPoints;
UniformBuffer::UniformBuffer(size_t size, void* data)
    : BufferCore(GL_UNIFORM_BUFFER, allocateBindPoint(), size, data) { }
UniformBuffer::~UniformBuffer() {
    allocatedBindPoints.erase(bufferBindPoint);
}

GLint UniformBuffer::allocateBindPoint() {
    if (allocatedBindPoints.size() == static_cast<size_t>(MaxBuffers())) {
        THROW VisAssert("Uniform Buffer Bindings exceeded!\nLimit = %d\n\nsdl_exp UniformBuffer objs are not designed for sharing buffer bindings.", MaxBuffers());
    }
    for (unsigned int i = static_cast<size_t>(MaxBuffers()) - 1; i >= 0; --i) {
        if (allocatedBindPoints.find(i) == allocatedBindPoints.end()) {
            allocatedBindPoints.insert(i);
            return i;
        }
    }
    THROW VisAssert("UniformBuffer::allocateBindPoint(): This point should never be reached!");
}

GLint UniformBuffer::MaxSize() {
    return BufferCore::maxSize(GL_UNIFORM_BUFFER);
}
GLint UniformBuffer::MaxBuffers() {
    return BufferCore::maxBuffers(GL_UNIFORM_BUFFER);
}
}  // namespace visualiser
}  // namespace flamegpu

// Comment out this include if not making use of Shaders/ShaderCore
#include "flamegpu/visualiser/shader/Shaders.h"
#ifdef SRC_FLAMEGPU_VISUALISER_SHADER_SHADERS_H_
namespace flamegpu {
namespace visualiser {
bool Shaders::setMaterialBuffer(const std::shared_ptr<UniformBuffer> &buffer) {
    return addBuffer(MATERIAL_UNIFORM_BLOCK_NAME, buffer);
}

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_SHADER_SHADERS_H_
