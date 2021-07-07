#ifndef SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_POINTLIGHT_H_
#define SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_POINTLIGHT_H_

#include "flamegpu/visualiser/shader/ShaderHeader.h"

namespace flamegpu {
namespace visualiser {

class LightsBuffer;

/**
 * Denotes pointlight by setting spotCosCutoff to invalid range spotCosCutoff < 0
 */
class PointLight {
 protected:
    friend class LightsBuffer;
    inline PointLight(LightProperties * const props, LightsBuffer::TLightProperties * const tProps, unsigned int index, bool init = true);
    /**
     * Direct write access to  position in light buffer
     */
    LightProperties * const properties;
    LightsBuffer::TLightProperties * const tProperties;
    const unsigned int index;

 public:
    inline PointLight(const PointLight &old);
    /**
     * The light's index within the buffer
     */
    unsigned int Index() const { return index;  }
    /**
     * The position of the light in world space
     */
    inline void Position(const glm::vec3 &xyz);
    glm::vec3 Position() const { return tProperties->position; }
    /**
     * Convenience method for setting a single colour light
     * @param rgb Full colour of light
     * @param ambiFactor Multiplied by rgb prior to setting Ambient colour
     * @param diffFactor Multiplied by rgb prior to setting Diffuse colour
     * @param specFactor Multiplied by rgb prior to setting Specular colour
     */
    inline void Color(const glm::vec3 &rgb, const float &ambiFactor, const float &diffFactor, const float &specFactor);
    /**
     * The ambient component of the light's colour
     * @note Default value (0, 0, 0) [Black]
     */
    inline void Ambient(const glm::vec3 &rgb);
    glm::vec3 Ambient() const { return properties->ambient; }
    /**
     * The diffuse component of the light's colour
     * @note Default value (1, 1, 1) [White]
     */
    inline void Diffuse(const glm::vec3 &rgb);
    glm::vec3 Diffuse() const { return properties->diffuse; }
    /**
     * The specular component of the light's colour
     * @note Default value (1, 1, 1) [White]
     */
    inline void Specular(const glm::vec3 &rgb);
    glm::vec3 Specular() const { return properties->specular; }
    /**
     * Attenuation is calculated as 1/(k_{c} + k_{l}d + k_{q}d^{2})
     */
    /**
     * Constant attenuation factor k_{c}
     * @note Default value 1
     * @note The inverse of this value is used, higher values lead to weaker lights
     */
    inline void ConstantAttenuation(const float &factor);
    float ConstantAttenuation() const { return properties->constantAttenuation; }
    /**
     * Linear attenuation factor k_{l}
     * @note Default value 0
     */
    inline void LinearAttenuation(const float &factor);
    float LinearAttenuation() const { return properties->linearAttenuation; }
    /**
     * Quadratic attenuation factor k_{q}
     * @note Default value 0
     */
    inline void QuadraticAttenuation(const float &factor);
    float QuadraticAttenuation() const { return properties->quadraticAttenuation; }
};

}  // namespace visualiser
}  // namespace flamegpu

#include "PointLight.inl"

#endif  // SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_POINTLIGHT_H_
