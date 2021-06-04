#ifndef SRC_MULTIPASS_RENDERBUFFER_H_
#define SRC_MULTIPASS_RENDERBUFFER_H_
#include <memory>

#include <glm/vec2.hpp>

#include "util/GLcheck.h"
#include "interface/RenderTarget.h"

namespace flamegpu {
namespace visualiser {

/**
 * Represents an OpenGL renderbuffer
 * These can be rendered to and blit, but not sampled like texture's
 */
class RenderBuffer : public RenderTarget {
 public:
    /**
     * Returns a new renderbuffer of the given dimensions and format
     * @param dimensions Size of the renderbuffer
     * @param internalFormat Internal format type of the renderbuffer
     * @param samples The number of samples, if 0 is passed multisampling will be disabled
     */
    static std::shared_ptr<RenderBuffer> make(glm::uvec2 dimensions, GLenum internalFormat, unsigned int samples = 0);
    ~RenderBuffer();
    /**
     * Resizes the renderbuffer
     * @param dimensions The new size of the renderbuffer
     */
    void resize(const glm::uvec2 &dimensions) override;
    /**
     * Returns the OpenGL name as allocated by glGenRenderbuffers()
     */
    GLuint getName() const override { return name; }
    /**
     * Copy constructor, creates a mutable copy
     */
    RenderBuffer(const RenderBuffer& b);
    /**
     * Move constructor and assignment operators disabled, contains const elements
     */
    RenderBuffer(const RenderBuffer&& b) = delete;
    RenderBuffer& operator= (const RenderBuffer& b) = delete;
    RenderBuffer& operator= (const RenderBuffer&& b) = delete;

 private:
    /**
     * private constructor
     * @see make()
     */
    RenderBuffer(glm::uvec2 dimensions, GLenum internalFormat, unsigned int samples);
    /**
    * Calls glGenRenderbuffers() returning the generated texture name
    */
    static GLuint genName();
    const GLuint name;
    const unsigned int samples;
    const GLenum internalFormat;
    glm::uvec2 dimensions;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_MULTIPASS_RENDERBUFFER_H_
