#ifndef SRC_FLAMEGPU_VISUALISER_TEXTURE_TEXTURE2D_MULTISAMPLE_H_
#define SRC_FLAMEGPU_VISUALISER_TEXTURE_TEXTURE2D_MULTISAMPLE_H_
#include <memory>

#include "Texture.h"
#include "flamegpu/visualiser/interface/RenderTarget.h"

namespace flamegpu {
namespace visualiser {

class Texture2D_Multisample : public Texture, public RenderTarget {
 public:
    static std::shared_ptr<Texture2D_Multisample> make(
        const glm::uvec2 &dimensions,
        const Texture::Format &format,
        const unsigned int &samples,
        const uint64_t &options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT);

    // Cant fill multi sample texture from host so don't allow copy (can be done with compute shader?)
    Texture2D_Multisample(const Texture2D_Multisample& b) = delete;
    Texture2D_Multisample(const Texture2D_Multisample&& b) = delete;
    Texture2D_Multisample& operator=(const Texture2D_Multisample& b) = delete;
    Texture2D_Multisample& operator=(const Texture2D_Multisample&& b) = delete;
    /**
     * Resizes the texture, retaining the existing number of samples
     * @param _dimensions New texture dimensions
     */
    void resize(const glm::uvec2 &_dimensions) override { resize(_dimensions, 0); }
    /**
     * Resizes the texture
     * @param dimensions New texture dimensions
     * @param samples The number of samples (if 0, the previous value will be used)
     */
    void resize(const glm::uvec2 &dimensions, unsigned int samples);
    /**
     * @return The dimensions of the currently alocated texture
     */
    glm::uvec2 getDimensions() const { return dimensions; }
    /**
     * @return The width of the currently alocated texture
     */
    unsigned int getWidth() const { return dimensions.x; }
    /**
     * @return The height of the currently alocated texture
     */
    unsigned int getHeight() const { return dimensions.y; }
    /**
     * @return boolean representing whether the texture is currently correct bound to it's allocated texture unit
     * @note This does not check whether it is the currently bound buffer!
     */
    bool isBound() const override;
    /**
     * Provides for RenderTarget's virtual getName(), simply defers call to Texture::getName();
     * @return The gl texture name as returned by glGenTextures()
     */
    GLenum getName() const override { return Texture::getName(); }

 private:
    /**
     * Private constructor
     * @see make(...)
     */
    Texture2D_Multisample(
        const glm::uvec2 &dimensions,
        const Texture::Format &format,
        const unsigned int &samples = 4,
        const uint64_t &options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT);
    /**
     * 2D Multisample specific allocation method
     * @param dimensions Dimensions of the texture to be allocated
     * @param samples The number of samples
     */
    void allocateMultisampleTextureMutable(const glm::uvec2 &dimensions, unsigned int samples = 0);
    /**
     * Used inside constructor to assign the instance a texture unit
     */
    static GLuint genTextureUnit();
    /**
     * Dimensions of the allocated texture
     */
    glm::uvec2 dimensions;
    static const char *RAW_TEXTURE_FLAG;
    unsigned int samples;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_TEXTURE_TEXTURE2D_MULTISAMPLE_H_
