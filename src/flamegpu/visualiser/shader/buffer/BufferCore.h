#ifndef SRC_FLAMEGPU_VISUALISER_SHADER_BUFFER_BUFFERCORE_H_
#define SRC_FLAMEGPU_VISUALISER_SHADER_BUFFER_BUFFERCORE_H_
#include "flamegpu/visualiser/util/GLcheck.h"

namespace flamegpu {
namespace visualiser {

/**
 * This class must be specialised e.g. GL_UNIFORM_BUFFER, GL_SHADER_STORAGE_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER or GL_ATOMIC_COUNTER_BUFFER (not yet implemented the latter two)
 * Representative of Uniform Buffer Objects
 * @note GL_TEXTURE_BUFFER are currently treated seperately as the pre-date gl4 buffers and have different binding procedures
 */
class BufferCore {
 public:
    /**
     * Returns the currently allocated size of the buffer
     */
    size_t getSize() const { return size; }
    /**
     * Returns the GL name of the allocated buffer
     * @note This permits you to manually handle the buffer
     */
    GLint getName() const { return bufferName; }
    /**
     * Returns the buffer binding point, this binding point is intended (by sdl_exp) to be unique to all buffers of the same type
     * @note glUniformBlockBinding() is used to bind the buffer (via it's binding point) to the shader uniform hosting the buffer
     */
    GLint getBufferBindPoint() const { return bufferBindPoint; }
    /**
     * Returns the GL buffer type, e.g. GL_UNIFORM_BUFFER
     */
    GLenum getType() const { return bufferType; }
    /*
     * Sets the data in the buffer, passing an alternate size can be used to resize the buffer
     * @param data Pointer to the data
     * @param size The amount of data to store in the buffer, 0 will use the existing size
     */
    void setData(void *data, size_t size = 0);
    /**
     * Passes data to a subset of the buffer, denoted by offset
     * @note The value of size+offset cannot exceed that returned by getSize()
     */
    void setData(void *data, size_t size, size_t offset);
    /**
     * Copies data out of the texture buffer
     * @param dataReturn Pointer to the destination that data will be copied
     * @param size The number of bytes to be copied. If 0 this defaults to the size of the buffer
     * @param offset The byte offset into the buffer. Defaults to 0
     */
    void getData(void *dataReturn, size_t size = 0, size_t offset = 0);
    /**
     * Provides read only access to a buffer
     * @note unmapBuffer() must be called after access is completed
     */
    void const *mapBufferRead();
    /**
     * Provides write only access to a buffer
     * @note unmapBuffer() must be called after access is completed
     */
    void *mapBufferWrite();
    /**
     * Provides read/write access to a buffer
     * @note unmapBuffer() must be called after access is completed
     */
    void *mapBufferReadWrite();
    /**
     * Unmaps a buffer
     */
    void unmapBuffer();
    /**
     * Returns the block type of the buffer.
     * e.g. GL_SHADER_STORAGE_BLOCK
     */
    GLenum getBlockType();

    static GLint maxSize(GLenum bufferType);
    static GLint maxBuffers(GLenum bufferType);
    // Do version for max per shader too, e.g. MAX_COMBINED_UNIFORM_BLOCKS, MAX_COMBINED_SHADER_STORAGE_BLOCKS

 protected:
    BufferCore(GLenum bufferType, GLint bindPoint, size_t bytes, void* data = nullptr);
    virtual ~BufferCore();

 private:
    /**
     * Provides access to a buffer
     * @param access The access level provided: GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE
     */
    void *mapBuffer(GLenum access);
    size_t size;
    GLuint bufferName;

 protected:
    const GLuint bufferBindPoint;
    const GLenum bufferType;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_SHADER_BUFFER_BUFFERCORE_H_
