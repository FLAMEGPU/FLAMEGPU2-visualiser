#include "flamegpu/visualiser/texture/Texture2D_Multisample.h"

#include <cstdio>
#include <memory>

namespace flamegpu {
namespace visualiser {


const char *Texture2D_Multisample::RAW_TEXTURE_FLAG = "Texture2D_Multisample";
std::shared_ptr<Texture2D_Multisample> Texture2D_Multisample::make(const glm::uvec2 &dimensions, const Texture::Format &format, const unsigned int &samples, const uint64_t &options) {
    return std::shared_ptr<Texture2D_Multisample>(new Texture2D_Multisample(dimensions, format, samples, options));
}
Texture2D_Multisample::Texture2D_Multisample(const glm::uvec2 &dimensions, const Texture::Format &format, const unsigned int &samples, const uint64_t &options)
    :Texture(GL_TEXTURE_2D_MULTISAMPLE, genTextureUnit(), format, RAW_TEXTURE_FLAG, options)
    , dimensions(dimensions)
    , samples(samples) {
    allocateMultisampleTextureMutable(dimensions, samples);
    applyOptions();
}
void Texture2D_Multisample::resize(const glm::uvec2 &_dimensions, unsigned int _samples) {
    this->dimensions = _dimensions;
    this->samples = _samples ? _samples : this->samples;
    allocateMultisampleTextureMutable(this->dimensions, this->samples);
}
void Texture2D_Multisample::allocateMultisampleTextureMutable(const glm::uvec2 &_dimensions, unsigned int _samples) {
    GL_CALL(glBindTexture(type, glName));
    GL_CALL(glTexImage2DMultisample(type, _samples, format.internalFormat, _dimensions.x, _dimensions.y, true));
    GL_CALL(glBindTexture(type, 0));
}
/**
* Required methods for handling texture units
*/
GLuint Texture2D_Multisample::genTextureUnit() {
    static GLuint texUnit = 1;
    GLint maxUnits;
    GL_CALL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits));  // 192 on Modern GPUs, spec minimum 80
#ifdef _DEBUG
    visassert(texUnit < static_cast<GLuint>(maxUnits));
#endif
    if (texUnit >= static_cast<GLuint>(maxUnits)) {
        texUnit = 1;
        fprintf(stderr, "Max texture units exceeded by GL_TEXTURE_2D_MULTISAMPLE, enable texture switching.\n");
        // If we ever notice this being triggered, need to add a static flag to Shaders which tells it to rebind textures to units at use.
        // Possibly even notifying it of duplicate units
    }
    return texUnit++;
}
bool Texture2D_Multisample::isBound() const {
    GL_CALL(glActiveTexture(GL_TEXTURE0 + textureUnit));
    GLint whichID;
    GL_CALL(glGetIntegerv(GL_TEXTURE_BINDING_2D_MULTISAMPLE, &whichID));
    return static_cast<GLuint>(whichID) == glName;
}

}  // namespace visualiser
}  // namespace flamegpu
