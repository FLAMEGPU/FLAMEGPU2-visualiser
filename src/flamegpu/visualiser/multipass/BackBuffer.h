#ifndef SRC_FLAMEGPU_VISUALISER_MULTIPASS_BACKBUFFER_H_
#define SRC_FLAMEGPU_VISUALISER_MULTIPASS_BACKBUFFER_H_
#include <memory>

#include <glm/glm.hpp>

#include "flamegpu/visualiser/interface/FBuffer.h"

namespace flamegpu {
namespace visualiser {

/**
 * Represents the default FrameBuffer, often called the BackBuffer in double-buffered rendering
 * FrontBuffer refers to the one currently shown, which is swapped with the BackBuffer after rendering has completed
 * @note This class simply wraps the binding of Buffer 0, and clearing
 * @note If clear is set, depth will always be cleared
 * @note Visualisations may override the backbuffer if they wish to redirect
 * This class would be purely static, however it can be used as an object for FBuffer compatibility
 */
class BackBuffer : public FBuffer {
    friend class ViewportExt;

 public:
    /**
     * @param doClear Whether to clear the frame on use()
     * @param clearColor The clear color (rgb) to be used
     * @note Out of bounds clearColor values will cause the static value to be used
     * @note Backbuffer is managed by the Visualisation, only whether the frame clear settings can be changed
     */
    explicit BackBuffer(const bool &doClear = true, const glm::vec3 &clearColor = glm::vec3(-1));
    /**
     * @return The name of the contained GL_FRAMEBUFFER
     * @note This it the value created by glGenFramebuffer() and used with glBindBuffer()
     */
    GLuint getFrameBufferName() final { return NAME; }
    /**
     * Binds the framebuffer
     * @return True if the framebuffer is 'complete' and was bound
     * @note This forwards calls to the static BackBuffer:useStatic()
     */
    bool use() final;
    /**
     * Binds the framebuffer
     * @return True if the framebuffer is 'complete' and was bound
     * @note This will use the global clear setting, not that one unique to any instances
     */
    static bool useStatic();
    /**
     * Enables/Disables the clear color, and sets the color (optional)
     * @param doClear Toggle whether the BackBuffer should clear the frame on use
     * @param clearColor The clear color (rgb) to be used
     * @note This is a global setting, shared between all instances of BackBuffer and useStatic();
     * @note Invalid clear colors will not be set (values must be in the inclusive range 0-1)
     */
    static void setClear(const bool &doClear, const glm::vec3 &clearColor = glm::vec3(-1));
    /**
     * Disabled by BackBuffer
     * Visualisations manage this value
     */
    void resize(const glm::uvec2 &dims) final;
    /**
     * The native BackBuffer has the GLName 0
     */
    static const GLuint NAME;

 protected:
    /**
     * @param dims The new viewport dimensions
     * Resizes the internal images according to the specifed dimensions and the internal scaling factor
     */
    static void resizeViewport(const glm::uvec2 &dims);
    /**
     * Allows Visualisations to provide an override which replaces the BackBuffer until cleared
     * @param _overrideBuffer Weak pointer to the buffer to be overriden
     * @note If the weak pointer becomes deallocated, the override will revert to the native back buffer
     */
    static void setOverride(std::weak_ptr<FBuffer> _overrideBuffer) { BackBuffer::overrideBuffer = _overrideBuffer; }
    /**
     * Clears the back buffer override
     */
    static void clearOverride() { BackBuffer::overrideBuffer.reset(); }

 private:
    /**
     * The internal use method, that lets static/non-static use() to pass the relevant clear
     */
    static bool _useStatic(const bool &doClear, const glm::vec3 &clearColor);
    static glm::uvec2 dimensions;
    static bool _doClear;
    static glm::vec3 _clearColor;
    static std::weak_ptr<FBuffer> overrideBuffer;

    bool doClear;
    glm::vec3 clearColor;
    bool useStaticClear;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_MULTIPASS_BACKBUFFER_H_
