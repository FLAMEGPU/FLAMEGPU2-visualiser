#ifndef SRC_MULTIPASS_FRAMEBUFFERATTACHMENT_H_
#define SRC_MULTIPASS_FRAMEBUFFERATTACHMENT_H_
#include <memory>

#include "util/GLcheck.h"
#include "interface/RenderTarget.h"
#include "texture/Texture.h"
#include "multipass/RenderBuffer.h"


/**
 * Interface to define the shared FBA members
 * These should be constructed via FBAFactory
 */
class FrameBufferAttachment {
    friend class FBAFactory;

 public:
    enum AttachClass { Null, Color, Depth, Stencil, DepthStencil };
    enum AttachType { Disabled, TextureRT, RenderBufferRT };
    AttachClass Class() const { return cls; }
    AttachType Type() const { return type; }
    GLint InternalFormat() const { return internalFormat; }
    GLenum PixelFormat() const { return pixelFormat; }
    GLenum StorageType() const { return storageType; }
    std::shared_ptr<::RenderTarget> RenderTarget() const { return rt; }
    virtual ~FrameBufferAttachment() {}

 private:
    FrameBufferAttachment(
        AttachClass cls,
        AttachType type,
        GLint internalFormat,
        GLenum pixelFormat,
        GLenum storageType,
        std::shared_ptr<::RenderTarget> tex)
        : cls(cls)
        , type(type)
        , internalFormat(internalFormat)
        , pixelFormat(pixelFormat)
        , storageType(storageType)
        , rt(tex) { }
    /**
     * Which class of attachment is it: Color, Depth, Stencil or DepthStencil
     */
    const AttachClass cls;
    /**
     * How to provide the attachement
     * @note You probably want Disabled or Texture, You can't sample from a renderbuffer
     */
    const AttachType type;
    /**
     * The internal format of the texture
     * @note Required for both texture and renderbuffer types
     * @see glTexImage2D() and glRenderbufferStorage()
     */
    const GLint internalFormat;  // GL_RGB, GL_RGBA8
    /**
     * The color format of the texture
     * @note Only required for texture types
     * @see glTexImage2D()
     */
    const GLenum pixelFormat;  // GL_RGB, GL_BGRA
    /**
     * The storage type of the texture
     * @note Only required for texture types
     * @see glTexImage2D()
     */
    const GLenum storageType;  // GL_UNSIGNED_BYTE
    /**
     * The name of the unmanaged texture or renderbuffer to use as a render target
     * @note Only set this value to something other than nullptr if you want to use an unmanaged texture
     * @see This is the value which glGenTextures() creates
     */
    const std::shared_ptr<::RenderTarget> rt;
};
class FBAFactory {
 public:
    // // // // // // // // // // // // // //
    //       Disabled FBA's    //
    // // // // // // // // // // // // // //
    /**
     * Defines a FBA capable of representing Color/Depth/Stencil/DepthStencil FBAs which are to be disabled
     */
    static FrameBufferAttachment Disabled() {
        return FrameBufferAttachment(FrameBufferAttachment::Null, FrameBufferAttachment::Disabled, 0, 0, 0, 0);
    }
    // // // // // // // // // // // // // //
    //       Color FBA's       //
    // // // // // // // // // // // // // //
    /**
     * Defines a FBA capable of representing color texture FBAs
     * @param internalFormat The internal format of the texture. e.g. GL_RGB, GL_RGBA
     * @param format The pixel storage format of the texture. e.g. GL_RGB, GL_RGBA, GL BGRA
     * @param storageType The data type of the pixel data
     * @see glTexImage2D() or glTexImage2DMultisample() for the full range of appropriate parameter configurations
     */
    static FrameBufferAttachment ManagedColorTexture(GLint internalFormat, GLenum format, GLenum storageType = GL_UNSIGNED_BYTE) {
        return FrameBufferAttachment(FrameBufferAttachment::Color, FrameBufferAttachment::TextureRT, internalFormat, format, storageType, nullptr);
    }
    /**
     * Defines a FBA capable of representing a RGB color texture FBA using a default configuration
     */
    static FrameBufferAttachment ManagedColorTextureRGB() {
        return ManagedColorTexture(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
    }
    /**
     * Defines a FBA capable of representing a RGBA color texture FBA using a default configuration
     */
    static FrameBufferAttachment ManagedColorTextureRGBA() {
        return ManagedColorTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    }
    /**
     * Defines a FBA capable of representing color renderbuffer FBAs
     * @param internalFormat The internal format of the renderbuffer. e.g. GL_RGB, GL_RGBA
     * @see glRenderbufferStorage() or glRenderbufferStorageMultisample() for the full range of appropriate parameter configurations
     */
    static FrameBufferAttachment ManagedColorRenderBuffer(GLint internalFormat) {
        return FrameBufferAttachment(FrameBufferAttachment::Color, FrameBufferAttachment::RenderBufferRT, internalFormat, 0, 0, nullptr);
    }
    /**
     * Defines a FBA capable of representing a RGB color renderbuffer FBA using a default configuration
     */
    static FrameBufferAttachment ManagedColorRenderBufferRGB() {
        return ManagedColorRenderBuffer(GL_RGB);
    }
    /**
     * Defines a FBA capable of representing a RGBA color renderbuffer FBA using a default configuration
     */
    static FrameBufferAttachment ManagedColorRenderBuffereRGBA() {
        return ManagedColorRenderBuffer(GL_RGBA);
    }
    ////////////////////////////
    //       Depth FBA's      //
    ////////////////////////////
    /**
     * Defines a FBA capable of representing depth texture FBAs
     * @param internalFormat The internal format of the texture. e.g. GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F.
     * @param storageType The data type of the pixel data
     * @see glTexImage2D() or glTexImage2DMultisample() for the full range of appropriate parameter configurations
     * @note GL_DEPTH_COMPONENT is unique in being able to utilise 'shadow' texture lookup functions
     */
    static FrameBufferAttachment ManagedDepthTexture(GLint internalFormat = GL_DEPTH_COMPONENT32F, GLenum storageType = GL_UNSIGNED_BYTE) {  // https:// www.khronos.org/opengles/sdk/docs/man3/docbook4/xhtml/glTexImage2D.xml
        if (internalFormat == GL_DEPTH_COMPONENT32F) {
            storageType = GL_UNSIGNED_SHORT;
        } else if (internalFormat == GL_DEPTH_COMPONENT24) {
            storageType = GL_UNSIGNED_INT;
        } else if (internalFormat == GL_DEPTH_COMPONENT24) {
            storageType = GL_FLOAT;
        }
        return FrameBufferAttachment(FrameBufferAttachment::Depth, FrameBufferAttachment::TextureRT, internalFormat, GL_DEPTH_COMPONENT, storageType, nullptr);
    }
    /**
     * Defines a FBA capable of representing depth renderbuffer FBAs
     * @param internalFormat The internal format of the texture. e.g. GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F.
     * @see glTexImage2D() or glTexImage2DMultisample() for the full range of appropriate parameter configurations
     */
    static FrameBufferAttachment ManagedDepthRenderBuffer(GLint internalFormat = GL_DEPTH_COMPONENT32F) {
        return FrameBufferAttachment(FrameBufferAttachment::Depth, FrameBufferAttachment::RenderBufferRT, internalFormat, 0, 0, nullptr);
    }
    // // // // // // // // // // // // // //
    //      Stencil FBA's      //
    // // // // // // // // // // // // // //
    /**
     * Defines a FBA capable of representing 8 bit stencil texture FBAs
     * @note It is also possible to use GL_STENCIL_INDEX1, GL_STENCIL_INDEX4 and GL_STENCIL_INDEX16. However the GL Wiki highly discourages this
     */
    static FrameBufferAttachment ManagedStencilTexture() {
        return FrameBufferAttachment(FrameBufferAttachment::Stencil, FrameBufferAttachment::TextureRT, GL_STENCIL_INDEX8, GL_STENCIL_INDEX8, GL_UNSIGNED_INT_24_8, nullptr);
    }
    /**
     * Defines a FBA capable of representing 8 bit stencil renderbuffer FBAs
     * @note It is also possible to use GL_STENCIL_INDEX1, GL_STENCIL_INDEX4 and GL_STENCIL_INDEX16. However the GL Wiki highly discourages this
     */
    static FrameBufferAttachment ManagedStencilRenderBuffer() {
        return FrameBufferAttachment(FrameBufferAttachment::Stencil, FrameBufferAttachment::RenderBufferRT, GL_STENCIL_INDEX8, 0, 0, nullptr);
    }
    // // // // // // // // // // // // // //
    //    DepthStencil FBA's   //
    // // // // // // // // // // // // // //
    /**
     * Defines a FBA capable of representing combined 24 bit depth, 8 bit stencil texture FBAs
     */
    static FrameBufferAttachment ManagedDepthStencilTexture24() {  // https:// www.khronos.org/opengles/sdk/docs/man3/docbook4/xhtml/glTexImage2D.xml
        return FrameBufferAttachment(FrameBufferAttachment::DepthStencil, FrameBufferAttachment::TextureRT, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    }
    /**
     * Defines a FBA capable of representing combined 32 bit depth, 8 bit stencil texture FBAs
     */
    static FrameBufferAttachment ManagedDepthStencilTexture32() {  // https:// www.khronos.org/opengles/sdk/docs/man3/docbook4/xhtml/glTexImage2D.xml
        return FrameBufferAttachment(FrameBufferAttachment::DepthStencil, FrameBufferAttachment::TextureRT, GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, nullptr);
    }
    /**
     * Defines a FBA capable of representing combined 24 bit depth, 8 bit stencil renderbuffer FBAs
     */
    static FrameBufferAttachment ManagedDepthStencilRenderBuffer24() {
        return FrameBufferAttachment(FrameBufferAttachment::DepthStencil, FrameBufferAttachment::RenderBufferRT, GL_DEPTH24_STENCIL8, 0, 0, nullptr);
    }
    /**
     * Defines a FBA capable of representing combined 32 bit depth, 8 bit stencil renderbuffer FBAs
     */
    static FrameBufferAttachment ManagedDepthStencilRenderBuffer32() {
        return FrameBufferAttachment(FrameBufferAttachment::DepthStencil, FrameBufferAttachment::RenderBufferRT, GL_DEPTH32F_STENCIL8, 0, 0, nullptr);
    }
    // // // // // // // // // // // // // //
    //     Unmanaged FBA's     //
    // // // // // // // // // // // // // //
    /**
     * Defines an unmanaged color texture FBA
     * The named texture will not be managed by sdl_exp
     */
    static FrameBufferAttachment UnmanagedColorTexture(std::shared_ptr<::RenderTarget> tex) {
        assert(std::dynamic_pointer_cast<Texture>(tex));
        return FrameBufferAttachment(FrameBufferAttachment::Color, FrameBufferAttachment::TextureRT, 0, 0, 0, tex);
    }
    /**
     * Defines an unmanaged depth texture FBA
     * The named texture will not be managed by sdl_exp
     */
    static FrameBufferAttachment UnmanagedDepthTexture(std::shared_ptr<::RenderTarget> tex) {
        assert(std::dynamic_pointer_cast<Texture>(tex));
        return FrameBufferAttachment(FrameBufferAttachment::Depth, FrameBufferAttachment::TextureRT, 0, 0, 0, tex);
    }
    /**
     * Defines an unmanaged stencil texture FBA
     * The named texture will not be managed by sdl_exp
     */
    static FrameBufferAttachment UnmanagedStencilTexture(std::shared_ptr<::RenderTarget> tex) {
        assert(std::dynamic_pointer_cast<Texture>(tex));
        return FrameBufferAttachment(FrameBufferAttachment::Stencil, FrameBufferAttachment::TextureRT, 0, 0, 0, tex);
    }
    /**
     * Defines an unmanaged depthstencil texture FBA
     * The named texture will not be managed by sdl_exp
     */
    static FrameBufferAttachment UnmanagedDepthStencilTexture(std::shared_ptr<::RenderTarget> tex) {
        assert(std::dynamic_pointer_cast<Texture>(tex));
        return FrameBufferAttachment(FrameBufferAttachment::DepthStencil, FrameBufferAttachment::TextureRT, 0, 0, 0, tex);
    }
    /**
     * Defines an unmanaged color renderbuffer FBA
     * The named renderbuffer will not be managed by sdl_exp
     */
    static FrameBufferAttachment UnmanagedColorRenderBuffer(std::shared_ptr<RenderBuffer> renderBuffer) {
        return FrameBufferAttachment(FrameBufferAttachment::Color, FrameBufferAttachment::RenderBufferRT, 0, 0, 0, renderBuffer);
    }
    /**
     * Defines an unmanaged depth renderbuffer FBA
     * The named renderbuffer will not be managed by sdl_exp
     */
    static FrameBufferAttachment UnmanagedDepthRenderBuffer(std::shared_ptr<RenderBuffer> renderBuffer) {
        return FrameBufferAttachment(FrameBufferAttachment::Depth, FrameBufferAttachment::RenderBufferRT, 0, 0, 0, renderBuffer);
    }
    /**
     * Defines an unmanaged stencil renderbuffer FBA
     * The named renderbuffer will not be managed by sdl_exp
     */
    static FrameBufferAttachment UnmanagedStencilRenderBuffer(std::shared_ptr<RenderBuffer> renderBuffer) {
        return FrameBufferAttachment(FrameBufferAttachment::Stencil, FrameBufferAttachment::RenderBufferRT, 0, 0, 0, renderBuffer);
    }
    /**
     * Defines an unmanaged depth stencil renderbuffer FBA
     * The named renderbuffer will not be managed by sdl_exp
     */
    static FrameBufferAttachment UnmanagedDepthStencilRenderBuffer(std::shared_ptr<RenderBuffer> renderBuffer) {
        return FrameBufferAttachment(FrameBufferAttachment::DepthStencil, FrameBufferAttachment::RenderBufferRT, 0, 0, 0, renderBuffer);
    }
};
#endif  // SRC_MULTIPASS_FRAMEBUFFERATTACHMENT_H_
