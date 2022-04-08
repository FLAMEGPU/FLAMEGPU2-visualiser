#ifndef SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_LIGHTSBUFFER_H_
#define SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_LIGHTSBUFFER_H_
#include "flamegpu/visualiser/shader/buffer/UniformBuffer.h"
#include "flamegpu/visualiser/shader/ShaderHeader.h"
// #include "flamegpu/visualiser/shader/lights/PointLightModel.h"
// #include "flamegpu/visualiser/shader/lights/SpotLightModel.h"

namespace flamegpu {
namespace visualiser {

class PointLight;
class SpotLight;
class DirectionalLight;

/**
 * This class acts as a wrapper over Uniform Buffer which stores lights
 * You can add and get existing lights via the contained methods
 * It is recommended that you store the returned lights in your own data structure
 * They can be created and cast via the copy constructor, but will always refer to their index in the parent LightsBuffer
 */
class LightsBuffer : private UniformBuffer {
 public:
    /**
     * Struct to hold temp properties
     */
    struct TLightProperties {
        TLightProperties()
            : spotCutoff(180.0f)
            , position(0, 0, 1, 1)
            , spotDirection(0, 0, -1, 0) { }
        // Stored in radians
        float spotCutoff;
        glm::vec4 position;
        glm::vec4 spotDirection;
    };
    explicit LightsBuffer(const glm::mat4 *viewMatPtr = nullptr);
    /**
     * Increments the number of lights and returns the newest light as a point light
     * @throws runtime_error If the number of lights would exceed MAX_LIGHTS
     */
    PointLight addPointLight();
    /**
     * Increments the number of lights and returns the newest light as a spot light
     * @throws runtime_error If the number of lights would exceed MAX_LIGHTS
     */
    SpotLight addSpotLight();
    /**
     * Increments the number of lights and returns the newest light as a directional light
     * @throws runtime_error If the number of lights would exceed MAX_LIGHTS
     */
    DirectionalLight addDirectionalLight();
    /**
     * Returns the light at the given index in the buffer as a point light
     * @throws runtime_error if index does not point to a current light
     * @note If you request a light which has already been created as a spotlight
     * you will simply lose access to spotlight specific variables via this instance
     */
    PointLight getPointLight(unsigned int index);
    /**
     * Returns the light at the given index in the buffer as a spot light
     * @throws runtime_error if index does not point to a current light
     */
    SpotLight getSpotLight(unsigned int index);
    /**
     * Returns the light at the given index in the buffer as a directional light
     * @throws runtime_error if index does not point to a current light
     */
    DirectionalLight getDirectionalLight(unsigned int index);
    /**
     * Asks the buffer to update the GPU copy if necessary
     * Computes eye transform of position/spotDirection
     * Could go further and negate spotDirection,
     */
    void update();
    /**
     * Returns the number of allocated lights
     */
    unsigned int getCount() const { return uniformBlock.lightsCount; }
    /**
     * Returns the maximum number of lights that can be allocated
     */
    static unsigned int getMaxCount() { return MAX_LIGHTS; }
    using BufferCore::getName;
    using BufferCore::getType;
    using BufferCore::getBufferBindPoint;
    /**
     * Sets the pointer from which the View matrix should be loaded from
     * @param viewMat A pointer to the viewMatrix to be tracked
     * @note This pointer is likely provided by a Camera subclass
     */
    void setViewMatPtr(const glm::mat4  *viewMat) { viewMatPtr = viewMat; }
    /**
     * Sets the pointer from which the projection matrix should be loaded from
     * @param projMat A pointer to the projection matrix to be tracked
     * @note This is only required to be set if render() will be called
     */
    void setProjectionMatPtr(const glm::mat4  *projMat) { projMatPtr = projMat; }
    // void setRenderScale(const float &scale);
    // void render();

 private:
    LightUniformBlock uniformBlock;
    TLightProperties tProperties[MAX_LIGHTS];
    const glm::mat4 *viewMatPtr;
    const glm::mat4 *projMatPtr;
    // void initModels();
    // std::unique_ptr<PointLightModel> modelPointLight;
    // std::unique_ptr<SpotLightModel> modelSpotLight;
};

}  // namespace visualiser
}  // namespace flamegpu

#include "PointLight.h"
#include "SpotLight.h"
#include "DirectionalLight.h"

#endif  // SRC_FLAMEGPU_VISUALISER_SHADER_LIGHTS_LIGHTSBUFFER_H_
