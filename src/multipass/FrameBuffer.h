#ifndef SRC_MULTIPASS_FRAMEBUFFER_H_
#define SRC_MULTIPASS_FRAMEBUFFER_H_

#include <map>
#include <unordered_set>
#include <memory>

#include <glm/glm.hpp>

#include "interface/FBuffer.h"
#include "multipass/FrameBufferAttachment.h"

namespace flamegpu {
namespace visualiser {

class Texture;
class RenderBuffer;

/**
 * This class represents a Framebuffer with custom 2D texture and renderbuffer attachments
 * The class supports multisampling, however that requires you to instead use Sampler2DMS to sample any textures inside shaders
 * If a scaling framebuffer is used, it will be resized whenever the viewport dimensions are changed (this includes resizing unmanaged textures/renderbuffers)
 * You can use Shaders::setFragOutAttribute() to bind the attachment point to a named output
 * @todo Improve stencilbuffer (control when writing/clearing/using) // https:// en.wikipedia.org/wiki/Stencil_buffer
 * @todo Add support for cubemap framebuffers
 */
class FrameBuffer : public FBuffer {
 public:
    // // // // // // // // // // // // // //
    //   Scaling FrameBuffers  //
    // // // // // // // // // // // // // //
    FrameBuffer(FrameBufferAttachment color, FrameBufferAttachment depth, FrameBufferAttachment stencil, unsigned int samples = 0, float scale = 1.0f, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    FrameBuffer(FrameBufferAttachment color, FrameBufferAttachment depthstencil, unsigned int samples = 0, float scale = 1.0f, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    FrameBuffer(std::initializer_list<FrameBufferAttachment> color, FrameBufferAttachment depth, FrameBufferAttachment stencil, unsigned int samples = 0, float scale = 1.0f, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    FrameBuffer(std::initializer_list<FrameBufferAttachment> color, FrameBufferAttachment depthstencil, unsigned int samples = 0, float scale = 1.0f, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    // // // // // // // // // // // // // //
    //    Fixed FrameBuffers   //
    // // // // // // // // // // // // // //
    FrameBuffer(glm::uvec2 dimensions, FrameBufferAttachment color, FrameBufferAttachment depth, FrameBufferAttachment stencil, unsigned int samples = 0, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    FrameBuffer(glm::uvec2 dimensions, FrameBufferAttachment color, FrameBufferAttachment depthstencil, unsigned int samples = 0, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    FrameBuffer(glm::uvec2 dimensions, std::initializer_list<FrameBufferAttachment> color, FrameBufferAttachment depth, FrameBufferAttachment stencil, unsigned int samples = 0, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    FrameBuffer(glm::uvec2 dimensions, std::initializer_list<FrameBufferAttachment> color, FrameBufferAttachment depthstencil, unsigned int samples = 0, bool doClear = true, glm::vec3 clearColor = glm::vec3(0));
    ~FrameBuffer();
    /**
     * @return Whether the framebuffer is 'complete' and ready to be rendered to
     */
    bool isValid() const;
    /**
     * Resizes the internal images according to the specifed dimensions and the internal scaling factor
     * @param dims The new viewport width and height
     */
    void resize(const glm::uvec2 &dims) final;
    /**
     * Binds the framebuffer
     * @return True if the framebuffer is 'complete' and was bound
     */
    bool use() final;
    /**
     * @return The name of the contained GL_FRAMEBUFFER
     */
    GLuint getFrameBufferName() override { return name; };
    /**
     * @param attachPt The attachment point required, these are 0-indexed in the order color attachments were bound
     * @return The name of the texture bound to the specified attachment point
     * @note 0 is returned if not bound or bound as renderbuffer
     */
    GLuint getColorTextureName(GLuint attachPt = 0) const;
    /**
     * @return The name of the texture bound to the specified attachment point
     * @note 0 is returned if not bound or bound as renderbuffer
     * @note This will return a value if you bound a Depth texture or a DepthStencil texture
     */
    GLuint getDepthTextureName() const;
    /**
     * @return The name of the texture bound to the specified attachment point
     * @note 0 is returned if not bound or bound as renderbuffer
     * @note This will return a value if you bound a Stencil texture or a DepthStencil texture
     */
    GLuint getStencilTextureName() const;
    /**
     * @return The name of the texture bound to the specified attachment point
     * @note 0 is returned if not bound or bound as renderbuffer
     * @note This will only return a value if you bound a DepthStencil texture
     */
    GLuint getDepthStencilTextureName() const;
    /**
     * @param attachPt The attachment point required, these are 0-indexed in the order color attachments were bound
     * @return The name of the renderbuffer bound to the specified attachment point
     * @note 0 is returned if not bound or bound as texture
     */
    GLuint getColorRenderBufferName(GLuint attachPt = 0) const;
    /**
     * @return The name of the renderbuffer bound to the specified attachment point
     * @note 0 is returned if not bound or bound as texture
     * @note This will return a value if you bound a Depth renderbuffer or a DepthStencil renderbuffer
     */
    GLuint getDepthRenderBufferName() const;
    /**
     * @return The name of the renderbuffer bound to the specified attachment point
     * @note 0 is returned if not bound or bound as texture
     * @note This will return a value if you bound a Stencil renderbuffer or a DepthStencil renderbuffer
     */
    GLuint getStencilRenderBufferName() const;
    /**
     * @return The name of the renderbuffer bound to the specified attachment point
     * @note 0 is returned if not bound or bound as texture
     * @note This will only return a value if you bound a DepthStencil renderbuffer
     */
    GLuint getDepthStencilRenderBufferName() const;
    /**
     * @param attachPt The attachment point required, these are 0-indexed in the order color attachments were bound
     * @return The texture bound to the specified attachment point
     * @note An empty shared_ptr is returned if not bound or bound as renderbuffer
     */
    std::shared_ptr<Texture> getColorTexture(GLuint attachPt = 0) const;
    /**
     * @return The texture bound to the specified attachment point
     * @note An empty shared_ptr is returned if not bound or bound as renderbuffer
     * @note This will return a value if you bound a Depth texture or a DepthStencil texture
     */
    std::shared_ptr<Texture> getDepthTexture() const;
    /**
     * @return The texture bound to the specified attachment point
     * @note An empty shared_ptr is returned if not bound or bound as renderbuffer
     * @note This will return a value if you bound a Stencil texture or a DepthStencil texture
     */
    std::shared_ptr<Texture> getStencilTexture() const;
    /**
     * @return The texture bound to the specified attachment point
     * @note An empty shared_ptr is returned if not bound or bound as renderbuffer
     * @note This will only return a value if you bound a DepthStencil texture
     */
    std::shared_ptr<Texture> getDepthStencilTexture() const;
    /**
     * @param attachPt The attachment point required, these are 0-indexed in the order color attachments were bound
     * @return The renderbuffer bound to the specified attachment point
     * @note An empty shared_ptr is returned if not bound or bound as texture
     */
    std::shared_ptr<RenderBuffer> getColorRenderBuffer(GLuint attachPt = 0) const;
    /**
     * @return The renderbuffer bound to the specified attachment point
     * @note 0 is returned if not bound or bound as texture
     * @note This will return a value if you bound a Depth renderbuffer or a DepthStencil renderbuffer
     */
    std::shared_ptr<RenderBuffer> getDepthRenderBuffer() const;
    /**
     * @return The renderbuffer bound to the specified attachment point
     * @note An empty shared_ptr is returned if not bound or bound as texture
     * @note This will return a value if you bound a Stencil renderbuffer or a DepthStencil renderbuffer
     */
    std::shared_ptr<RenderBuffer> getStencilRenderBuffer() const;
    /**
     * @return The renderbuffer bound to the specified attachment point
     * @note An empty shared_ptr is returned if not bound or bound as texture
     * @note This will only return a value if you bound a DepthStencil renderbuffer
     */
    std::shared_ptr<RenderBuffer> getDepthStencilRenderBuffer() const;
    /**
     * @return The number of samples
     * @note 0 Means that multisampling for the FrameBuffer is disabled
     */
    unsigned int getSampleCount() const { return samples; }
    /**
     * @return The current dimensions of the FrameBuffer
     * @note If this is a scaling FrameBuffer this value may change over time
     */
    glm::uvec2 getDimensions() const { return dimensions; }
    /**
     * Sets whether the Framebuffer is to be automatically cleared before use
     */
    void setDoClear(bool _doClear) { this->doClear = _doClear; }
    /**
     * @return Whether the Framebuffer is to be automatically cleared before use
     */
    bool getDoClear() const { return doClear; }
    /**
     * Sets the clear color to be used
     */
    void setClearColor(glm::vec3 _clearColor) { this->clearColor = glm::vec4(_clearColor, 1.0f); }
    void setClearColor(glm::vec4 _clearColor) { this->clearColor = _clearColor; }
    /**
     * @return The clear color to be used
     */
    glm::vec4 getClearColor() const { return clearColor; }
    /**
     * Disables filtering for the specified color attachment's texture
     * @param attachPt The attachment point to disable filtering for, default 0 (aka single color texture)
     */
    void disableFiltering(GLuint attachPt = 0);

 private:
    inline GLenum GL_TEXTURE_TYPE() const { return samples == 0 ? GL_TEXTURE_2D : GL_TEXTURE_2D_MULTISAMPLE; }
    /**
     * Internal constructor, triggered by public ones
     */
    FrameBuffer(
        std::initializer_list<FrameBufferAttachment> color,
        FrameBufferAttachment depth,
        FrameBufferAttachment stencil,
        FrameBufferAttachment depthstencil,
        float scale, glm::uvec2 dimensions,
        unsigned int samples,
        glm::vec3 clearColor,
        bool doClear);
    /**
     * Does all the heavy lifting for generating attachments
     */
    void makeAttachment(const FrameBufferAttachment &attachmentConfig, GLenum attachPoint, std::shared_ptr<RenderTarget> &renderTarget);
    /**
     * Generates and resizes all attachments
     */
    void makeAttachments();
    /**
     * Scale of images held by this framebuffer, with respect to the viewport
     * If the scale <= 0, Then the framebuffer has fixed dimensions
     */
    const float scale;
    /**
     * Current dimensions of images held by this framebuffer
     */
    glm::uvec2 dimensions;
    /**
     * The number of samples used, if the value is 0 multisampling will be disabled
     */
    unsigned int samples;
    /**
     * The name of the framebuffer, as set by glGenFramebuffers()
     */
    GLuint name;
    /**
     * The clear colour to clear the frame with prior to rendering
     */
    glm::vec4 clearColor;
    /**
     * Whether to clear the frame prior to rendering
     */
    bool doClear;
    struct ConfNamePair {
        explicit ConfNamePair(FrameBufferAttachment fbaConf)
            : conf(fbaConf), renderTarget(nullptr) { }
        /**
         * The frame buffer attachment configuration
         */
        const FrameBufferAttachment conf;
        /**
         * The GL texture/renderbuffer for the attachment
         */
        std::shared_ptr<RenderTarget> renderTarget;
    };
    /**
     * Config for each color attachment
     * Key: Color attachment point
     * Value: Color attachment configuration/name
     */
    std::map<GLuint, ConfNamePair> colors;
    /**
     * Config for the depth stencil attachment
     */
    ConfNamePair depthStencil;
    /**
     * Config for the stencil attachment
     */
    ConfNamePair depth;
    /**
     * Config for the stencil attachment
     */
    ConfNamePair stencil;
    /**
     * Notifies the framebuffer which color attachments are used
     * @note This method can only be called once per framebuffer
     */
    void setDrawBuffers();
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_MULTIPASS_FRAMEBUFFER_H_
