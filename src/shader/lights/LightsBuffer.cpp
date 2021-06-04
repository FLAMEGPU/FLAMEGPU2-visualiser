#include "shader/lights/LightsBuffer.h"

#include <memory>
#include <stdexcept>

#include "shader/lights/PointLight.h"
#include "shader/lights/SpotLight.h"
#include "shader/lights/DirectionalLight.h"

namespace flamegpu {
namespace visualiser {

LightsBuffer::LightsBuffer(const glm::mat4 *viewMatPtr)
    : UniformBuffer(MAX_LIGHTS*sizeof(LightProperties))
    , tProperties()
    , viewMatPtr(viewMatPtr)
    , projMatPtr(nullptr) {
    uniformBlock.lightsCount = 0;
}
PointLight LightsBuffer::addPointLight() {
    if (uniformBlock.lightsCount < MAX_LIGHTS) {
        unsigned int index = uniformBlock.lightsCount++;
        return PointLight(&uniformBlock.lights[index], &tProperties[index], index);
    }
    THROW VisAssert("LightsBuffer::addPointLight(): Max lights (%u) exceeded in light buffer.\n", MAX_LIGHTS);
}
SpotLight LightsBuffer::addSpotLight() {
    if (uniformBlock.lightsCount < MAX_LIGHTS) {
        unsigned int index = uniformBlock.lightsCount++;
        return SpotLight(&uniformBlock.lights[index], &tProperties[index], index);
    }
    THROW VisAssert("LightsBuffer::addSpotLight(): Max lights (%u) exceeded in light buffer.\n", MAX_LIGHTS);
}
DirectionalLight LightsBuffer::addDirectionalLight() {
    if (uniformBlock.lightsCount < MAX_LIGHTS) {
        unsigned int index = uniformBlock.lightsCount++;
        return DirectionalLight(&uniformBlock.lights[index], &tProperties[index], index);
    }
    THROW VisAssert("LightsBuffer::addDirectionalLight(): Max lights (%u) exceeded in light buffer.\n", MAX_LIGHTS);
}
PointLight LightsBuffer::getPointLight(unsigned int index) {
    if (index < uniformBlock.lightsCount) {
        return PointLight(&uniformBlock.lights[index], &tProperties[index], index, false);
    }
    THROW VisAssert("LightsBuffer::getSpotLight(): Light index '%u' is invalid.\n", index);
}
SpotLight LightsBuffer::getSpotLight(unsigned int index) {
    if (index < uniformBlock.lightsCount) {
        return SpotLight(&uniformBlock.lights[index], &tProperties[index], index, false);
    }
    THROW VisAssert("LightsBuffer::getSpotLight(): Light index '%u' is invalid.\n", index);
}
DirectionalLight LightsBuffer::getDirectionalLight(unsigned int index) {
    if (index < uniformBlock.lightsCount) {
        return DirectionalLight(&uniformBlock.lights[index], &tProperties[index], index, false);
    }
    THROW VisAssert("LightsBuffer::getDirectionalLight(): Light index '%u' is invalid.\n", index);
}
void LightsBuffer::update() {
    static bool once = true;
    if (viewMatPtr) {
        // Transform light values into eye space
        for (unsigned int i = 0; i < uniformBlock.lightsCount; ++i) {
            uniformBlock.lights[i].position = (*viewMatPtr) * tProperties[i].position;
            uniformBlock.lights[i].spotDirection = (*viewMatPtr) * tProperties[i].spotDirection;
        }
    } else {
        if (once) {
            fprintf(stderr, "Warning: viewMatPtr has not been passed to LightsBuffer, lights will not be transformed to eye space.\n");
            once = false;
        }
        for (unsigned int i = 0; i < uniformBlock.lightsCount; ++i) {
            uniformBlock.lights[i].position = tProperties[i].position;
            uniformBlock.lights[i].spotDirection = tProperties[i].spotDirection;
        }
    }
    setData(&uniformBlock, sizeof(glm::vec4) + (uniformBlock.lightsCount * sizeof(LightProperties)));  // sizeof(LightUniformBlock)
}

}  // namespace visualiser
}  // namespace flamegpu

// Comment out this include if not making use of Shaders/ShaderCore
#include "interface/Renderable.h"

#ifdef SRC_INTERFACE_RENDERABLE_H_
namespace flamegpu {
namespace visualiser {

void Renderable::setLightsBuffer(std::shared_ptr<const LightsBuffer> buffer) {  // Treat it similar to texture binding points
    setLightsBuffer(buffer->getBufferBindPoint());
}

}  // namespace visualiser
}  // namespace flamegpu

#endif
