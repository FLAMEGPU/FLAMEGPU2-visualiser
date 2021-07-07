#ifndef SRC_FLAMEGPU_VISUALISER_MULTIPASS_RENDERPASS_H_
#define SRC_FLAMEGPU_VISUALISER_MULTIPASS_RENDERPASS_H_
#include <memory>

#include <glm/glm.hpp>

#include "flamegpu/visualiser/interface/FBuffer.h"

namespace flamegpu {
namespace visualiser {

/**
 * Base class that binds the named FrameBuffer before calling the subclasses render() to trigger rendering.
 */
class RenderPass {
 public:
    /**
     * Creates a RenderPass which renders to the default FrameBuffer (aka back buffer)
     * The resolution of the backbuffer is controlled by the viewport
     * @param clearColor The colour to clear the backBuffer with prior to render, each component is in the range 0-1
     * @param doClear Whether to clear prior to render
     */
    explicit RenderPass(glm::vec3 clearColor = glm::vec3(0), bool doClear = true);
    /**
     * Creates a Renderpass which renders to a custom FrameBuffer
     */
    explicit RenderPass(std::shared_ptr<FBuffer> fb);
    /**
     * Allow subclasses to be destroyed properly by RenderPass ptrs
     */
    virtual ~RenderPass() { }
    std::shared_ptr<FBuffer> getFrameBuffer() { return fb; }
    /**
     * Binds the contained FrameBuffer and then triggers render()
     * If the FrameBuffer is incomplete (invalid), the render will be skipped and the previous framebuffer bound
     * @note This method is called by the scene in order of RenderPass priority
     */
    virtual void executeRender();
    /**
     * Triggered when the viewport resizes, such that attached textures can be auto resized
     */
    void resize(const unsigned int &width, const unsigned int &height) { resize({width, height}); }
    virtual void resize(const glm::uvec2 &dims);

 private:
    const std::shared_ptr<FBuffer> fb;

 protected:
    /**
     * Called by executeRender() after framebuffer has been bound
     */
    virtual void render() = 0;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_MULTIPASS_RENDERPASS_H_
