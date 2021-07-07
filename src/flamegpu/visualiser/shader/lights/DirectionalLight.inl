#ifndef SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_DIRECTIONALLIGHT_INL_
#define SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_DIRECTIONALLIGHT_INL_

#include "DirectionalLight.h"
#include "flamegpu/visualiser/shader/lights/LightsBuffer.h"

namespace flamegpu {
namespace visualiser {

inline DirectionalLight::DirectionalLight(const PointLight &old)
    : PointLight(old) { }

inline DirectionalLight::DirectionalLight(LightProperties * const props, LightsBuffer::TLightProperties * const tProps, unsigned int index, bool init)
    : PointLight(props, tProps, index, init) {
    if (init) {
        tProperties->spotCutoff = glm::radians(45.0f);  // We don't care, this value should never be used
        properties->spotCosCutoff = 2.0f;  // Signifies directional light
        properties->constantAttenuation = 1.0f;  // Directional light only has constant attenuation
        properties->linearAttenuation = 0.0f;  // Directional light only has constant attenuation
        properties->quadraticAttenuation = 0.0f;  // Directional light only has constant attenuation
    }
}

inline void DirectionalLight::Direction(const glm::vec3 &dir) {
    tProperties->spotDirection = glm::vec4(glm::normalize(dir), 0.0f);
}

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_DIRECTIONALLIGHT_INL_
