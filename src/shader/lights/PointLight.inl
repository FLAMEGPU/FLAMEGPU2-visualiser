#ifndef SRC_SHADER_LIGHTS_POINTLIGHT_INL_
#define SRC_SHADER_LIGHTS_POINTLIGHT_INL_

#include "shader/lights/PointLight.h"
#include "shader/lights/LightsBuffer.h"

namespace flamegpu {
namespace visualiser {

PointLight::PointLight(const PointLight &old)
    : properties(old.properties)
    , tProperties(old.tProperties)
    , index(old.index) { }

PointLight::PointLight(LightProperties * const props, LightsBuffer::TLightProperties * const tProps, unsigned int index, bool init)
    : properties(props)
    , tProperties(tProps)
    , index(index) {
    if (init) {
        properties[0] = LightProperties();  // Initialise light
        tProperties->spotCutoff = 180.0f;  // This value should never be used, but just incase it evaluates to -1.0f
        properties->spotCosCutoff = -1.0f;  // Negative signifies this is point light
    }
}

void PointLight::Color(const glm::vec3 &rgb, const float &ambiFactor, const float &diffFactor, const float &specFactor) {
    properties->ambient = ambiFactor * rgb;
    properties->diffuse = diffFactor * rgb;
    properties->specular = specFactor * rgb;
}

void PointLight::Position(const glm::vec3 &xyz) {
    tProperties->position = glm::vec4(xyz, 1.0f);
}
void PointLight::Ambient(const glm::vec3 &rgb) {
    properties->ambient = rgb;
}
void PointLight::Diffuse(const glm::vec3 &rgb) {
    properties->diffuse = rgb;
}
void PointLight::Specular(const glm::vec3 &rgb) {
    properties->specular = rgb;
}
void PointLight::ConstantAttenuation(const float &factor) {
    properties->constantAttenuation = factor;
}
void PointLight::LinearAttenuation(const float &factor) {
    properties->linearAttenuation = factor;
}
void PointLight::QuadraticAttenuation(const float &factor) {
    properties->quadraticAttenuation = factor;
}

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_SHADER_LIGHTS_POINTLIGHT_INL_
