#include "multipass/BackBuffer.h"
#include <glm/gtx/component_wise.hpp>

namespace flamegpu {
namespace visualiser {

std::weak_ptr<FBuffer> BackBuffer::overrideBuffer;
glm::uvec2 BackBuffer::dimensions = glm::uvec2(1280, 720);
bool BackBuffer::_doClear = true;
glm::vec3 BackBuffer::_clearColor = glm::vec3(0);  // Black
const GLuint BackBuffer::NAME = 0;

BackBuffer::BackBuffer(const bool &doClear, const glm::vec3 &clearColor)
    : doClear(doClear)
    , clearColor(clearColor)
    , useStaticClear(compMin(clearColor) < 0 || compMax(clearColor) > 1.0f) { }
void BackBuffer::resize(const glm::uvec2 &) {
    static bool warn = false;
    if (!warn) {
        warn = true;
        fprintf(stderr, "Users cannot resize the backbuffer, this is managed by the Visualisation.\n");
    }
}
bool BackBuffer::use() {
    return BackBuffer::_useStatic(doClear, useStaticClear ? _clearColor : clearColor);
}
void BackBuffer::resizeViewport(const glm::uvec2 &dims) {
    if (dims.x > 0 && dims.y > 0)
        BackBuffer::dimensions = dims;
}
bool BackBuffer::useStatic() {
    return _useStatic(_doClear, _clearColor);
}
bool BackBuffer::_useStatic(const bool &doClear, const glm::vec3 &clearColor) {
    if (auto o = overrideBuffer.lock()) {
        if (o->use())
            return true;
        fprintf(stderr, "BackBuffer override failed, reverting to FrameBuffer 0.\n");
    }
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, NAME));
    if (doClear) {
        GL_CALL(glClearColor(clearColor.x, clearColor.y, clearColor.z, 1));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }
    GL_CALL(glViewport(0, 0, dimensions.x, dimensions.y));
    // The backbuffer cannot be modified, therefore it must always be complete.
    return true;
}
void BackBuffer::setClear(const bool& doClear, const glm::vec3& clearColor) {
    BackBuffer::_doClear = doClear;
    if (compMin(clearColor) >= 0 && compMax(clearColor) <= 1.0f)
        BackBuffer::_clearColor = clearColor;
}

}  // namespace visualiser
}  // namespace flamegpu
