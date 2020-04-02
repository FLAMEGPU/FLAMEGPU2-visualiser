#include "multipass/RenderBuffer.h"

std::shared_ptr<RenderBuffer> RenderBuffer::make(glm::uvec2 dimensions, GLenum internalFormat, unsigned int samples) {
    return std::shared_ptr<RenderBuffer>(new RenderBuffer(dimensions, internalFormat, samples));
}

RenderBuffer::RenderBuffer(glm::uvec2 dimensions, GLenum internalFormat, unsigned int samples)
    : name(genName())
    , samples(samples)
    , internalFormat(internalFormat)
    , dimensions(dimensions) {
    RenderBuffer::resize(dimensions);
}
RenderBuffer::RenderBuffer(const RenderBuffer& b)
    :RenderBuffer(b.dimensions, b.internalFormat, b.samples) { }
void RenderBuffer::resize(const glm::uvec2 &_dimensions) {
    this->dimensions = _dimensions;
    // Set storage
    GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, name));
    if (samples > 1) {
        GL_CALL(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat, dimensions.x, dimensions.y));
    } else {
        GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, dimensions.x, dimensions.y));
    }
    GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
}
RenderBuffer::~RenderBuffer() {
    GL_CALL(glDeleteRenderbuffers(1, &name));
}
GLuint RenderBuffer::genName() {
    GLuint name = 0;
    GL_CALL(glGenRenderbuffers(1, &name));
    return name;
}
