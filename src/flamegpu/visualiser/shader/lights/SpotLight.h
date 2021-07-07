#ifndef SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_SPOTLIGHT_H_
#define SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_SPOTLIGHT_H_
#include "PointLight.h"

namespace flamegpu {
namespace visualiser {

/**
 * Denotes spotlight by setting spotCosCutoff to valid range 0 <=spotCosCutoff <=1
 * @note Protected override to prevent unwanted casting
 */
class SpotLight : protected PointLight {
 protected:
    friend class LightsBuffer;
    inline SpotLight(LightProperties * const props, LightsBuffer::TLightProperties * const tProps, unsigned int index, bool init = true);

 public:
    explicit inline SpotLight(const PointLight &old);
    /**
     * The direction that the spotlight faces
     * This automatically normalises the value
     * @note Default value (0, 0, -1)
     */
    inline void Direction(const glm::vec3 &dir);
    glm::vec3 Direction() const { return tProperties->spotDirection; }
    /**
     * The angle of the spotlight's beam in degrees
     * This value is valid from 0-90 inclusive
     * @note Default value 45.0f degrees
     */
    inline void CutOff(const float &degrees);
    float CutOff() const { return tProperties->spotCutoff; }
    float CosCutOff() const { return properties->spotCosCutoff; }
    /**
     * The light's intensity is highest in the center of the cone.
     * It's attenuated toward the edges of the cone by the cosine of the angle between
     * the direction of the light and the direction from the light to the vertex being lit,
     * raised to the power of the spot exponent.
     * Thus, higher spot exponents result in a more focused light source.
     * Visible if you use a value like 50, with constant attenuation of 1
     * @note Default value 0.0f
     */
    inline void Exponent(const float &exponent);
    float Exponent() const { return properties->spotExponent; }

    // Shared properties
    using PointLight::Index;
    using PointLight::Position;
    using PointLight::Color;
    using PointLight::Ambient;
    using PointLight::Diffuse;
    using PointLight::Specular;
    using PointLight::ConstantAttenuation;
    using PointLight::LinearAttenuation;
    using PointLight::QuadraticAttenuation;
};

}  // namespace visualiser
}  // namespace flamegpu

#include "SpotLight.inl"
#endif  // SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_SPOTLIGHT_H_
