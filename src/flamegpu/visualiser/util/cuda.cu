#include "flamegpu/visualiser/util/cuda.h"


#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#include <cstring>
#include <typeinfo>   //  operator typeid
#include <stdexcept>


namespace flamegpu {
namespace visualiser {

// Drop in replacement if CUDA_CALL is missing
#ifndef CUDA_CALL
#define CUDA_CALL(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line) {
    if (code != cudaSuccess) {
        // TODO: Exception with message?
        // THROW CUDAError("CUDA Error: %s(%d): %s", file, line, cudaGetErrorString(code));
        fprintf(stdout, "CUDA Error: %s(%d): %s", file, line, cudaGetErrorString(code));
        throw std::exception();
    }
}
#endif


/*
Hidden internal functions
*/
namespace {
/*
Internal function used to detect the required internal format
@param componentCount The number of components
@param a A garbage value to specify the desired type because specialised templates werent working
@return the internal format
@see https:// www.opengl.org/sdk/docs/man/html/glTexBuffer.xhtml
*/
GLenum _getInternalFormat(const unsigned int componentCount, float) {
    if (componentCount == 1) return GL_R32F;
    if (componentCount == 2) return GL_RG32F;
    if (componentCount == 3 || componentCount == 4) return GL_RGBA32F;
    return 0;
}
GLenum _getInternalFormat(const unsigned int componentCount, unsigned int) {
    if (componentCount == 1) return GL_R32UI;
    if (componentCount == 2) return GL_RG32UI;
    if (componentCount == 3 || componentCount == 4) return GL_RGBA32UI;
    return 0;
}
GLenum _getInternalFormat(const unsigned int componentCount, int) {
    if (componentCount == 1) return GL_R32I;
    if (componentCount == 2) return GL_RG32I;
    if (componentCount == 3 || componentCount == 4) return GL_RGBA32I;
    return 0;
}
/*
@param componentCount The number of components (1-2, 4). Passing 3 will be treated as 4
@param bufferSize The total size of the buffer in bytes
@param d_TexPointer A device pointer to the mapped texture buffer
@return The filled cudaResourDesc (memset to 0 if invalid inputs)
*/
template<class T>
cudaResourceDesc _getCUDAResourceDesc(const unsigned int componentCount, const unsigned int bufferSize, const T *d_TexPointer) {
    cudaResourceDesc resDesc;
    memset(&resDesc, 0, sizeof(cudaResourceDesc));
    // Return empty if invalid input
    if (d_TexPointer == 0 ||
        bufferSize == 0 ||
        componentCount == 0 ||
        componentCount > 4)
        return resDesc;
    // Linear because its a texture buffer, not a texture
    resDesc.resType = cudaResourceTypeLinear;
    // Mapped pointer to the texture buffer on the device
    resDesc.res.linear.devPtr = reinterpret_cast<void*>(const_cast<T*>(d_TexPointer));
    // The type of the components

    if (typeid(T) == typeid(float)) {
        resDesc.res.linear.desc.f = cudaChannelFormatKindFloat;
    } else if (typeid(T) == typeid(unsigned int)) {
        resDesc.res.linear.desc.f = cudaChannelFormatKindUnsigned;
    } else if (typeid(T) == typeid(int)) {
        resDesc.res.linear.desc.f = cudaChannelFormatKindSigned;
    }
    // The number of bits per component (0 if not used)
    resDesc.res.linear.desc.x = 32;
    if (componentCount >= 2) {
        resDesc.res.linear.desc.y = 32;
        if (componentCount >= 3) {
            resDesc.res.linear.desc.z = 32;
            resDesc.res.linear.desc.w = 32;
        }
    }
    // The total buffer size
    resDesc.res.linear.sizeInBytes = bufferSize;
    return resDesc;
}
}  // namespace

template<class T>
CUDATextureBuffer<T> *mallocGLInteropTextureBuffer(const unsigned int elementCount, const unsigned int t_componentCount) {
    if (elementCount == 0||
        t_componentCount == 0 ||
        t_componentCount > 4)
        return nullptr;
    // Temporary storage of return values
    GLuint glTexName;
    GLuint glTBO;
    T *d_MappedPointer = nullptr;
    cudaGraphicsResource_t cuGraphicsRes;
    cudaTextureObject_t cuTextureObj;

    // Interpretation of buffer type/component details
    const unsigned int componentCount = t_componentCount == 3 ? 4 : t_componentCount;
    const unsigned int componentSize = sizeof(T);
    const unsigned int elementSize = componentSize*componentCount;
    const unsigned int bufferSize = elementSize * elementCount;
    const GLuint internalFormat = _getInternalFormat(componentCount, static_cast<T>(0));

    // Gen tex
    GL_CALL(glGenTextures(1, &glTexName));
    // Gen buffer
    GL_CALL(glGenBuffers(1, &glTBO));
    // Size buffer and tie to tex
    GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, glTBO));
    GL_CALL(glBufferData(GL_TEXTURE_BUFFER, bufferSize, 0, GL_STATIC_DRAW));                                    // TODO dynamic draw better?

    GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, glTexName));
    GL_CALL(glTexBuffer(GL_TEXTURE_BUFFER, internalFormat, glTBO));
    GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, 0));
    GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, 0));

    // Get CUDA handle to texture
    CUDA_CALL(cudaGraphicsGLRegisterBuffer(&cuGraphicsRes, glTBO, cudaGraphicsMapFlagsNone));
    // Map/convert this to something cuGraphicsRes
    CUDA_CALL(cudaGraphicsMapResources(1, &cuGraphicsRes));
    CUDA_CALL(cudaGraphicsResourceGetMappedPointer(reinterpret_cast<void**>(&d_MappedPointer), 0, cuGraphicsRes));
    CUDA_CALL(cudaGraphicsUnmapResources(1, &cuGraphicsRes, 0));
    // Create a texture object from the cuGraphicsRes
    cudaResourceDesc resDesc = _getCUDAResourceDesc(componentCount, bufferSize, d_MappedPointer);
    cudaTextureDesc texDesc;
    memset(&texDesc, 0, sizeof(cudaTextureDesc));
    texDesc.readMode = cudaReadModeElementType;  // Read as actual type, other option is normalised float
    // texDesc.addressMode[0] = cudaAddressModeWrap;  // We can only affect the address mode for first 3 dimensions, so lets leave it default
    CUDA_CALL(cudaCreateTextureObject(&cuTextureObj, &resDesc, &texDesc, nullptr));
    // Copy the generated data
    return new CUDATextureBuffer<T>(glTexName, glTBO, d_MappedPointer, cuGraphicsRes, cuTextureObj, elementCount, componentCount);
}
template<class T>
void freeGLInteropTextureBuffer(CUDATextureBuffer<T> *texBuf) {
    CUDA_CALL(cudaDestroyTextureObject(texBuf->cuTextureObj));
    CUDA_CALL(cudaGraphicsUnregisterResource(texBuf->cuGraphicsRes));
    GL_CALL(glDeleteBuffers(1, &texBuf->glTBO));
    GL_CALL(glDeleteTextures(1, &texBuf->glTexName));
    delete texBuf;
}
/**
 * Returns true if cudaMemcpy returns cudaSuccess
 */
bool _cudaMemcpyDeviceToDevice(void* dst, const void* src, size_t count) {
    auto t = cudaMemcpy(dst, src, count, cudaMemcpyDeviceToDevice);
    CUDA_CALL(t);
    return t == cudaSuccess;
}
// Explicit instantiation of templates
template CUDATextureBuffer<float> *mallocGLInteropTextureBuffer(const unsigned int, const unsigned int);
template CUDATextureBuffer<int> *mallocGLInteropTextureBuffer(const unsigned int, const unsigned int);
template CUDATextureBuffer<unsigned int> *mallocGLInteropTextureBuffer(const unsigned int, const unsigned int);
template void freeGLInteropTextureBuffer(CUDATextureBuffer<float>*);
template void freeGLInteropTextureBuffer(CUDATextureBuffer<int>*);
template void freeGLInteropTextureBuffer(CUDATextureBuffer<unsigned int>*);

}  // namespace visualiser
}  // namespace flamegpu
