#ifndef SRC_FLAMEGPU_VISUALISER_UTIL_CUDA_H_
#define SRC_FLAMEGPU_VISUALISER_UTIL_CUDA_H_
#include <cstdint>

#include "flamegpu/visualiser/util/GLcheck.h"

namespace flamegpu {
namespace visualiser {

#ifndef __CUDACC__
// Borrowed from cuda headers
struct cudaGraphicsResource;
typedef struct cudaGraphicsResource *cudaGraphicsResource_t;
typedef uint64_t cudaTextureObject_t;  // actual header says unsigned long long, but linter doesn't like that
#else
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>
#endif

template<class T>
struct CUDATextureBuffer {
    CUDATextureBuffer(
        const GLuint glTexName,
        const GLuint glTBO,
        T *d_mappedPointer,
        const cudaGraphicsResource_t cuGraphicsRes,
        cudaTextureObject_t cuTextureObj,
        const unsigned int elementCount,
        const unsigned int componentCount
    )
        : glTexName(glTexName)
        , glTBO(glTBO)
        , d_mappedPointer(d_mappedPointer)
        , cuGraphicsRes(cuGraphicsRes)
        , cuTextureObj(cuTextureObj)
        , elementCount(elementCount)
        , componentCount(componentCount) { }
    const GLuint glTexName;
    const GLuint glTBO;
    T *d_mappedPointer;
    const cudaGraphicsResource_t cuGraphicsRes;  // These are typedefs over pointers, need to store the actual struct?
    const cudaTextureObject_t cuTextureObj;
    const unsigned int elementCount;
    const unsigned int componentCount;
};
/**
 * Allocates a GL_TEXTURE_BUFFER of the desired size and binds it for use with CUDA-GL interop
 * @param elementCount The number of elements in the texture buffer
 * @param componentCount The number of components per element (either 1, 2, 3 or 4, default 1)
 * @tparam T The type of the data to be stored in the texture buffer (either float, int or unsigned int)
 * @return The struct storing data related to the generated texture buffer (0 if invalid input)
 * @see freeGLInteropTextureBuffer(CUDATextureBuffer *)
 * @see http://docs.nvidia.com/cuda/cuda-runtime-api/group__CUDART__OPENGL.html#group__CUDART__OPENGL
 * @see https://www.opengl.org/sdk/docs/man/html/glTexBuffer.xhtml
 */
template<class T>
CUDATextureBuffer<T> *mallocGLInteropTextureBuffer(const unsigned int elementCount, const unsigned int componentCount = 1);
/**
 * Deallocates all data allocated by the matching call to mallocGLInteropTextureBuffer()
 * @param texBuf The texture buffer to be deallocated
 * @see mallocGLInteropTextureBuffer(const unsigned int, const unsigned int, const GLuint)
 */
template<class T>
void freeGLInteropTextureBuffer(CUDATextureBuffer<T> *texBuf);
/**
* Returns true if cudaMemcpy returns cudaSuccess
*/
bool _cudaMemcpyDeviceToDevice(void* dst, const void* src, size_t count);

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_UTIL_CUDA_H_
