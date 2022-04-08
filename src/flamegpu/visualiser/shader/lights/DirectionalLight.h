#ifndef SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_DIRECTIONALLIGHT_H_
#define SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_DIRECTIONALLIGHT_H_
#include "flamegpu/visualiser/shader/lights/PointLight.h"

namespace flamegpu {
namespace visualiser {

/**
 * Denotes a directional light by setting spotCosCutoff to invalid range spotCosCutoff>1
 * Stores direction in spotDirection, ignores position and hence only has constant attenuation
 * @note Protected override to prevent unwanted casting
 */
class DirectionalLight : protected PointLight {
 protected:
    friend class LightsBuffer;
    inline DirectionalLight(LightProperties * const props, LightsBuffer::TLightProperties * const tProps, unsigned int index, bool init = true);

 public:
    explicit inline DirectionalLight(const PointLight &old);
    /**
     * The direction that the light faces
     * This automatically normalises the value
     * @note Default value (0, 0, -1)
     */
    inline void Direction(const glm::vec3 &dir);
    glm::vec3 Direction() const { return tProperties->spotDirection; }

    // Shared properties
    using PointLight::Index;
    using PointLight::Color;
    using PointLight::Ambient;
    using PointLight::Diffuse;
    using PointLight::Specular;
    using PointLight::ConstantAttenuation;

 private:
    // Directional light needs no position, or fancy attenuation (as it has no position)
    using PointLight::Position;
    using PointLight::LinearAttenuation;
    using PointLight::QuadraticAttenuation;
};

}  // namespace visualiser
}  // namespace flamegpu

#include "flamegpu/visualiser/shader/lights/DirectionalLight.inl"
#endif  // SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_DIRECTIONALLIGHT_H_
