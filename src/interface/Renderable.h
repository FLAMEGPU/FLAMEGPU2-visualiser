#ifndef SRC_INTERFACE_RENDERABLE_H_
#define SRC_INTERFACE_RENDERABLE_H_
#include <memory>

#include <glm/glm.hpp>

#include "Reloadable.h"
#include "Viewport.h"
#include "interface/Camera.h"
#include "util/GLcheck.h"


class LightsBuffer;

/**
 * Represents things which hold shaders (and can be rendered)
 * @note Entities are the target audience
 */
class Renderable : public Reloadable {
 public:
    /**
     * Binds the provided modelview matrix to the internal shader
     * @param viewMat Ptr to modelview matrix
     * @note This is normally found within the Camera object
     */
    virtual void setViewMatPtr(const glm::mat4 *viewMat) = 0;
    /**
     * Binds the provided projection matrix to the internal shader
     * @param projectionMat Ptr to model view matrix
     * @note This is normally found within the Viewport object
     */
    virtual void setProjectionMatPtr(const glm::mat4 *projectionMat) = 0;
    /**
     * Binds the provided view matrix to the internal shader
     * @param camera Ptr to view matrix
     * @note This is normally found within the Camera object
     * @note This method is overriden by Skybox as it required modelview sans translation
     */
    virtual void setViewMatPtr(std::shared_ptr<const Camera> camera) {
        setViewMatPtr(camera->getViewMatPtr());
    }
    /**
     * Binds the provided projection matrix to the internal shader
     * @param visualisation Ptr to model view matrix
     * @note Convenience wrapper
     */
    void setProjectionMatPtr(const Viewport *visualisation) {
        setProjectionMatPtr(visualisation->getProjectionMatPtr());
    }
    virtual void setLightsBuffer(const GLuint &bufferBindingPoint) = 0;
    virtual void setLightsBuffer(std::shared_ptr<const LightsBuffer> buffer);
};

#endif  // SRC_INTERFACE_RENDERABLE_H_
