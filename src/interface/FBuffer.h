#ifndef SRC_INTERFACE_FBUFFER_H_
#define SRC_INTERFACE_FBUFFER_H_
#include "../util/GLcheck.h"
#include <glm/glm.hpp>

namespace flamegpu {
namespace visualiser {

/**
 * Frame buffer interface shared by FrameBuffer and BackBuffer classes
 * Named FBuffer because Framebuffer is better suited to the actual class than the interface
 * Alternatively might rename this FrameBufferInterface, but that feels a little long
 * @note Could internalise clearBuffer and active dimension tracking into this interface
 */
class FBuffer {
 public:
    /**
     * Virtual to permit proper destruction of subclasses
     */
    virtual ~FBuffer() {}
    /**
     * @return The name of the contained GL_FRAMEBUFFER
     * @note This it the value created by glGenFramebuffer() and used with glBindBuffer()
     */
    virtual GLuint getFrameBufferName() = 0;
    /**
     * @param width The new viewport width
     * @param height The new viewport height
     * Resizes the internal images according to the specifed dimensions and the internal scaling factor
     */
    void resize(const unsigned int &width, const unsigned int &height) { resize({ width, height }); }
    /**
     * @param dims The new viewport width and height
     * Resizes the internal images according to the specifed dimensions and the internal scaling factor
     */
    virtual void resize(const glm::uvec2 &dims) = 0;
    /**
    * Binds the framebuffer
    * @return True if the framebuffer is 'complete' and was bound
    */
    virtual bool use() = 0;
    /**
    * @return The currently bound framebuffer
    */
    static GLuint getActiveFB() {
        GLuint prevFB = 0;
        //  GL_FRAMEBUFFER_BINDING Enum has MANY names based on extension/version
        //  but they all map to 0x8CA6
        GL_CALL(glGetIntegerv(0x8CA6, reinterpret_cast<GLint*>(&prevFB)));
        return prevFB;
    }
    /**
     * Returns the maximum number of colour attachments permitted by drivers
     * @note 8 is the minimum required by the spec
     */
    static int getMaxColorAttachments() {
        int rtn = 0;
        GL_CALL(glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &rtn));
        return rtn;
    }
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_INTERFACE_FBUFFER_H_
