#include "multipass/FrameBuffer.h"

#include "texture/Texture2D.h"
#include "texture/Texture2D_Multisample.h"
#include "multipass/RenderBuffer.h"
// Constructors

typedef FrameBufferAttachment FBA;
// Scaling FrameBuffers
FrameBuffer::FrameBuffer(FBA color, FBA depth, FBA stencil, unsigned int samples, float scale, bool doClear, glm::vec3 clearColor)
    : FrameBuffer({ color }, depth, stencil, FBAFactory::Disabled(), scale, glm::uvec2(1), samples, clearColor, doClear) {}
FrameBuffer::FrameBuffer(FBA color, FBA depthstencil, unsigned int samples, float scale, bool doClear, glm::vec3 clearColor)
: FrameBuffer({ color }, FBAFactory::Disabled(), FBAFactory::Disabled(), depthstencil, scale, glm::uvec2(1), samples, clearColor, doClear) {}
FrameBuffer::FrameBuffer(std::initializer_list<FBA> color, FBA depth, FBA stencil, unsigned int samples, float scale, bool doClear, glm::vec3 clearColor)
: FrameBuffer(color, depth, stencil, FBAFactory::Disabled(), scale, glm::uvec2(1), samples, clearColor, doClear) {}
FrameBuffer::FrameBuffer(std::initializer_list<FBA> color, FBA depthstencil, unsigned int samples, float scale, bool doClear, glm::vec3 clearColor)
: FrameBuffer(color, FBAFactory::Disabled(), FBAFactory::Disabled(), depthstencil, scale, glm::uvec2(1), samples, clearColor, doClear) {}
// Fixed FrameBuffers
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, FBA color, FBA depth, FBA stencil, unsigned int samples, bool doClear, glm::vec3 clearColor)
    : FrameBuffer({ color }, depth, stencil, FBAFactory::Disabled(), 0, dimensions, samples, clearColor, doClear) {}
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, FBA color, FBA depthstencil, unsigned int samples, bool doClear, glm::vec3 clearColor)
    : FrameBuffer({ color }, FBAFactory::Disabled(), FBAFactory::Disabled(), depthstencil, 0, dimensions, samples, clearColor, doClear) {}
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, std::initializer_list<FBA> color, FBA depth, FBA stencil, unsigned int samples, bool doClear, glm::vec3 clearColor)
    : FrameBuffer(color, depth, stencil, FBAFactory::Disabled(), 0, dimensions, samples, clearColor, doClear) {}
FrameBuffer::FrameBuffer(glm::uvec2 dimensions, std::initializer_list<FBA> color, FBA depthstencil, unsigned int samples, bool doClear, glm::vec3 clearColor)
    : FrameBuffer(color, FBAFactory::Disabled(), FBAFactory::Disabled(), depthstencil, 0, dimensions, samples, clearColor, doClear) {}
// Actual constructor
FrameBuffer::FrameBuffer(std::initializer_list<FBA> color, FBA depth, FBA stencil, FBA depthstencil, float scale, glm::uvec2 dimensions, unsigned int samples, glm::vec3 clearColor, bool doClear)
    : scale(scale)
    , dimensions(dimensions)
    , samples(samples)
    , clearColor(glm::vec4(clearColor, 1.0f))
    , doClear(doClear)
    , depthStencil(depthstencil)
    , depth(depth)
    , stencil(stencil) {
#ifdef _DEBUG
    int r;
    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &r);
    assert(static_cast<unsigned int>(r) >= samples);
    glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &r);
    assert(static_cast<unsigned int>(r) >= samples);
    glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &r);
    assert(static_cast<unsigned int>(r) >= samples);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &r);
    assert(static_cast<unsigned int>(r) >= dimensions.x);
    assert(static_cast<unsigned int>(r) >= dimensions.y);
#endif
    if (!(depthstencil.Type() == FBA::Disabled || (depth.Type() == FBA::Disabled && stencil.Type() == FBA::Disabled))) {
        THROW VisAssert("FrameBuffer::FrameBuffer(): Invalid combination of depth, stencil and depthstencil FrameBufferAttachements.\n");
    }
    if (color.size() > static_cast<unsigned int>(getMaxColorAttachments())) {
        THROW VisAssert("FrameBuffer::FrameBuffer(): Number of colors (%u) exceeds max color attachments(%u).\n", color.size(), static_cast<unsigned int>(getMaxColorAttachments()));
    }
    GL_CALL(glGenFramebuffers(1, &name));     {  // Assign an attachment point to each color attachment
        unsigned int i = 0;
        for (FBA c : color) {
            if (c.Class() != FBA::Null) {
                colors.emplace(i++, c);
            }
        }
    }
    makeAttachments();
    setDrawBuffers();
}
FrameBuffer::~FrameBuffer() {
    // Color
    colors.clear();
    // DepthStencil
    depthStencil.renderTarget.reset();
    depthStencil.conf.RenderTarget().reset();
    depth.renderTarget.reset();
    depth.conf.RenderTarget().reset();
    stencil.renderTarget.reset();
    stencil.conf.RenderTarget().reset();
    // FrameBuffer
    GL_CALL(glDeleteFramebuffers(1, &name));
}
// Internal loaders
void FrameBuffer::makeAttachments() {
    for (auto &&it = colors.begin(); it != colors.end(); ++it) {
        if (it->second.conf.Class() != FBA::Color) {
            THROW VisAssert("FrameBuffer::makeAttachments(): FrameBufferAttachment type mismatch.\n");
        }
        makeAttachment(it->second.conf, GL_COLOR_ATTACHMENT0 + it->first, it->second.renderTarget);
    }
    if (depthStencil.conf.Type() != FBA::Disabled) {
        if (depthStencil.conf.Class() != FBA::DepthStencil) {
            THROW VisAssert("FrameBuffer::makeAttachments(): FrameBufferAttachment type mismatch.\n");
        }
        makeAttachment(depthStencil.conf, GL_DEPTH_STENCIL_ATTACHMENT, depthStencil.renderTarget);
    } else {
        if (depth.conf.Type() != FBA::Disabled) {
            if (depth.conf.Class() != FBA::Depth) {
                THROW VisAssert("FrameBuffer::makeAttachments(): FrameBufferAttachment type mismatch.\n");
            }
            makeAttachment(depth.conf, GL_DEPTH_ATTACHMENT, depth.renderTarget);
        }
        if (stencil.conf.Type() != FBA::Disabled) {
            if (stencil.conf.Class() != FBA::Stencil) {
                THROW VisAssert("FrameBuffer::makeAttachments(): FrameBufferAttachment type mismatch.\n");
            }
            makeAttachment(stencil.conf, GL_STENCIL_ATTACHMENT, stencil.renderTarget);
        }
    }
}
void FrameBuffer::makeAttachment(const FBA &attachmentConfig, GLenum attachPoint, std::shared_ptr<RenderTarget> &renderTarget) {
    if (attachmentConfig.Type() != FBA::Disabled) {
        GLuint prevFBO = getActiveFB();
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
        if (attachmentConfig.Type() == FBA::TextureRT) {
            if (!renderTarget) {  // If this is the first call, set the render target
                if (!attachmentConfig.RenderTarget()) {  // If texture is managed
                        Texture::Format fmt(attachmentConfig.PixelFormat(), attachmentConfig.InternalFormat(), 0, attachmentConfig.StorageType());  // Size is unused for our requirements, so 0 will suffice
                        renderTarget = samples
                            ? std::dynamic_pointer_cast<RenderTarget>(Texture2D_Multisample::make(dimensions, fmt, samples, nullptr,
                                Texture::WRAP_CLAMP_TO_EDGE | Texture::FILTER_MAG_LINEAR | Texture::FILTER_MIN_LINEAR | Texture::DISABLE_MIPMAP))
                            : std::dynamic_pointer_cast<RenderTarget>(Texture2D::make(dimensions, fmt, nullptr,
                                Texture::WRAP_CLAMP_TO_EDGE | Texture::FILTER_MAG_LINEAR | Texture::FILTER_MIN_LINEAR | Texture::DISABLE_MIPMAP));
                        // Bind the tex to our framebuffer
                        GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, attachPoint, GL_TEXTURE_TYPE(), renderTarget->getName(), 0));
                        return;  // No need to resize if we just created
                } else {  // Texture is unmanaged
                    // Ensure format is correct
                    if (samples) {
                        if (!std::dynamic_pointer_cast<Texture2D_Multisample>(attachmentConfig.RenderTarget())) {
                            THROW VisAssert("FrameBuffer::makeAttachment(): Attachment with samples must be type Texture2D_Multisample.\n");
                        }
                    } else {
                        if (!std::dynamic_pointer_cast<Texture2D>(attachmentConfig.RenderTarget())) {
                            THROW VisAssert("FrameBuffer::makeAttachment(): Attachment without samples must be type Texture2D.\n");
                        }
                    }
                    renderTarget = attachmentConfig.RenderTarget();
                    // Bind the tex to our framebuffer
                    GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachPoint, GL_RENDERBUFFER, renderTarget->getName()));
                }
            }
            // Resize
            renderTarget->resize(dimensions);
        } else if (attachmentConfig.Type() == FBA::RenderBufferRT) {
            if (!renderTarget) {  // If this is the first call, set the render target
                if (!attachmentConfig.RenderTarget()) {  // managed
                    renderTarget = RenderBuffer::make(dimensions, attachmentConfig.InternalFormat(), samples);
                    // Bind the to our framebuffer
                    GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachPoint, GL_RENDERBUFFER, renderTarget->getName()));
                    return;  // No need to resize if we just created
                } else {  // unmanaged
                    renderTarget = attachmentConfig.RenderTarget();
                    // Bind the to our framebuffer
                    GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachPoint, GL_RENDERBUFFER, renderTarget->getName()));
                }
            }
        }
        renderTarget->resize(dimensions);
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
    }
}
void FrameBuffer::setDrawBuffers() {
    GLuint prevFBO = getActiveFB();
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
    // Allocate array
    unsigned int itemCt = static_cast<unsigned int>(colors.size() > 0 ? colors.size() : 1);
    GLenum *drawBuffsArr = reinterpret_cast<GLenum *>(malloc(sizeof(GLenum) * itemCt));
    // Fill Array
    if (colors.size() > 0) {
        unsigned int i = 0;
        for (auto && it : colors) {
            drawBuffsArr[i] = GL_COLOR_ATTACHMENT0 + it.first;
            i++;
        }
    } else {
        drawBuffsArr[0] = GL_NONE;
    }
    // Pass to GL
    GL_CALL(glDrawBuffers(itemCt, drawBuffsArr));
    // Free Array
    free(drawBuffsArr);
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
}
// Functional methods
bool FrameBuffer::isValid() const {
    GLuint prevFBO = getActiveFB();
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
    GLint st = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
    return st == GL_FRAMEBUFFER_COMPLETE;
}
void FrameBuffer::resize(const glm::uvec2 &dims) {
    if (scale > 0) {
        dimensions = glm::uvec2(ceil(glm::vec2(dims)*scale));
        makeAttachments();
    }
}
bool FrameBuffer::use()  {
#if _DEBUG  // Only do this check in debug
    GLuint prevFBO = getActiveFB();
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
    GLint st = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (st != GL_FRAMEBUFFER_COMPLETE) {
        if (st == GL_FRAMEBUFFER_UNDEFINED) {
            printf("%s(%i) FBO Incomplete;\n%s\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_UNDEFINED", "FrameBuffer is default FrameBuffer, which doesn't exist.");
        } else if (st == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
            printf("%s(%i) FBO Incomplete;\n%s\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT", "Atleast one attachment point is nolonger valid or has size 0.");
        } else if (st == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
            printf("%s(%i) FBO Incomplete;\n%s\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT", "No images are attached to the framebuffer.");
        } else if (st == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER) {
            printf("%s(%i) FBO Incomplete;\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
        } else if (st == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER) {
            printf("%s(%i) FBO Incomplete;\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
        } else if (st == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
            printf("%s(%i) FBO Incomplete;\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
        } else if (st == GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS) {
            printf("%s(%i) FBO Incomplete;\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
        } else if (st == GL_FRAMEBUFFER_UNSUPPORTED) {
            printf("%s(%i) FBO Incomplete;\n%s\n%s\n", __FILE__, __LINE__, "GL_FRAMEBUFFER_UNSUPPORTED", "The combination of internal formats of the attached images violates an implementation-dependent set of restrictions..");
        } else {
            printf("%s(%i) FBO Incomplete;\n%s\n", __FILE__, __LINE__, "Unknown Issue.");
        }
        // Rebind prev buffer on failure
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, prevFBO));
        return false;
    }
#else
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, name));
#endif  // _DEBUG

    if (doClear) {
        GL_CALL(glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }
    GL_CALL(glViewport(0, 0, dimensions.x, dimensions.y));
    return true;
}
// Name Getters
GLuint FrameBuffer::getColorTextureName(GLuint attachPt) const {
    auto && it = colors.find(attachPt);
    if (it != colors.end() && it->second.conf.Type() == FBA::TextureRT)
        return it->second.renderTarget->getName();
    return 0;
}
GLuint FrameBuffer::getDepthTextureName() const {
    if (depth.conf.Type() == FBA::TextureRT)
        return depth.renderTarget->getName();
    if (depthStencil.conf.Type() == FBA::TextureRT)
        return depthStencil.renderTarget->getName();
    return 0;
}
GLuint FrameBuffer::getStencilTextureName() const {
    if (stencil.conf.Type() == FBA::TextureRT)
        return stencil.renderTarget->getName();
    if (depthStencil.conf.Type() == FBA::TextureRT)
        return depthStencil.renderTarget->getName();
    return 0;
}
GLuint FrameBuffer::getDepthStencilTextureName() const {
    if (depthStencil.conf.Type() == FBA::TextureRT)
        return depthStencil.renderTarget->getName();
    return 0;
}
GLuint FrameBuffer::getColorRenderBufferName(GLuint attachPt) const {
    auto && it = colors.find(attachPt);
    if (it != colors.end() && it->second.conf.Type() == FBA::RenderBufferRT)
        return it->second.renderTarget->getName();
    return 0;
}
GLuint FrameBuffer::getDepthRenderBufferName() const {
    if (depth.conf.Type() == FBA::RenderBufferRT)
        return depth.renderTarget->getName();
    if (depthStencil.conf.Type() == FBA::RenderBufferRT)
        return depthStencil.renderTarget->getName();
    return 0;
}
GLuint FrameBuffer::getStencilRenderBufferName() const {
    if (stencil.conf.Type() == FBA::RenderBufferRT)
        return stencil.renderTarget->getName();
    if (depthStencil.conf.Type() == FBA::RenderBufferRT)
        return depthStencil.renderTarget->getName();
    return 0;
}
GLuint FrameBuffer::getDepthStencilRenderBufferName() const {
    if (depthStencil.conf.Type() == FBA::RenderBufferRT)
        return depthStencil.renderTarget->getName();
    return 0;
}
// Ptr Getters
std::shared_ptr<Texture> FrameBuffer::getColorTexture(GLuint attachPt) const {
    auto && it = colors.find(attachPt);
    if (it != colors.end() && it->second.conf.Type() == FBA::TextureRT)
        return std::dynamic_pointer_cast<Texture>(it->second.renderTarget);
    return nullptr;
}
std::shared_ptr<Texture> FrameBuffer::getDepthTexture() const {
    if (depth.conf.Type() == FBA::TextureRT)
        return std::dynamic_pointer_cast<Texture>(depth.renderTarget);
    if (depthStencil.conf.Type() == FBA::TextureRT)
        return std::dynamic_pointer_cast<Texture>(depthStencil.renderTarget);
    return nullptr;
}
std::shared_ptr<Texture> FrameBuffer::getStencilTexture() const {
    if (stencil.conf.Type() == FBA::TextureRT)
        return std::dynamic_pointer_cast<Texture>(stencil.renderTarget);
    if (depthStencil.conf.Type() == FBA::TextureRT)
        return std::dynamic_pointer_cast<Texture>(depthStencil.renderTarget);
    return nullptr;
}
std::shared_ptr<Texture> FrameBuffer::getDepthStencilTexture() const {
    if (depthStencil.conf.Type() == FBA::TextureRT)
        return std::dynamic_pointer_cast<Texture>(depthStencil.renderTarget);
    return nullptr;
}
std::shared_ptr<RenderBuffer> FrameBuffer::getColorRenderBuffer(GLuint attachPt) const {
    auto && it = colors.find(attachPt);
    if (it != colors.end() && it->second.conf.Type() == FBA::RenderBufferRT)
        return std::dynamic_pointer_cast<RenderBuffer>(it->second.renderTarget);
    return nullptr;
}
std::shared_ptr<RenderBuffer> FrameBuffer::getDepthRenderBuffer() const {
    if (depth.conf.Type() == FBA::RenderBufferRT)
        return std::dynamic_pointer_cast<RenderBuffer>(depth.renderTarget);
    if (depthStencil.conf.Type() == FBA::RenderBufferRT)
        return std::dynamic_pointer_cast<RenderBuffer>(depthStencil.renderTarget);
    return nullptr;
}
std::shared_ptr<RenderBuffer> FrameBuffer::getStencilRenderBuffer() const {
    if (stencil.conf.Type() == FBA::RenderBufferRT)
        return std::dynamic_pointer_cast<RenderBuffer>(stencil.renderTarget);
    if (depthStencil.conf.Type() == FBA::RenderBufferRT)
        return std::dynamic_pointer_cast<RenderBuffer>(depthStencil.renderTarget);
    return nullptr;
}
std::shared_ptr<RenderBuffer> FrameBuffer::getDepthStencilRenderBuffer() const {
    if (depthStencil.conf.Type() == FBA::RenderBufferRT)
        return std::dynamic_pointer_cast<RenderBuffer>(depthStencil.renderTarget);
    return nullptr;
}
void FrameBuffer::disableFiltering(GLuint attachPt) {
    auto && it = colors.find(attachPt);
    if (it != colors.end() && it->second.conf.Type() == FBA::TextureRT) {
        std::dynamic_pointer_cast<Texture>(depth.renderTarget)->setOptions(Texture::FILTER_MIN_NEAREST | Texture::FILTER_MAG_NEAREST);
    }
}
