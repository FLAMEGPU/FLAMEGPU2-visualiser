#ifndef SRC_SHADER_LIGHTS_SPOTLIGHT_INL_
#define SRC_SHADER_LIGHTS_SPOTLIGHT_INL_

#include "shader/lights/SpotLight.h"
#include "shader/lights/LightsBuffer.h"

namespace flamegpu {
namespace visualiser {

inline SpotLight::SpotLight(const PointLight &old)
    : PointLight(old) { }

inline SpotLight::SpotLight(LightProperties * const props, LightsBuffer::TLightProperties * const tProps, unsigned int index, bool init)
    : PointLight(props, tProps, index, init) {
    if (init) {
        tProperties->spotCutoff = glm::radians(45.0f);  // Set the spotlight setting
        properties->spotCosCutoff = cos(tProperties->spotCutoff);
    }
}

inline void SpotLight::Direction(const glm::vec3 &dir) {
    tProperties->spotDirection = glm::vec4(glm::normalize(dir), 0.0f);
}
inline void SpotLight::CutOff(const float &degrees) {
    tProperties->spotCutoff = glm::radians(degrees);
    properties->spotCosCutoff = cos(tProperties->spotCutoff);
}
inline void SpotLight::Exponent(const float &exponent) {
    properties->spotExponent = exponent;
}

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_SHADER_LIGHTS_SPOTLIGHT_INL_
