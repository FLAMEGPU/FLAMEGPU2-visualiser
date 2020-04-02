#include "multipass/RenderPass.h"

#include "multipass/BackBuffer.h"

RenderPass::RenderPass(glm::vec3 clearColor, bool doClear)
    : fb(std::make_shared<BackBuffer>(doClear, clearColor)) { }

RenderPass::RenderPass(std::shared_ptr<FBuffer> fb)
    : fb(fb) { }
void RenderPass::executeRender() {
    // Skip render if Framebuffer is incomplete
    if (!fb->use())
        return;
    render();
}
void RenderPass::resize(const glm::uvec2 &dims) {
    if (!std::dynamic_pointer_cast<BackBuffer>(fb))
        fb->resize(dims);
}
