#ifndef SRC_INTERFACE_CAMERA_H_
#define SRC_INTERFACE_CAMERA_H_

#include <glm/glm.hpp>
/**
 * Represents the core components of a camera interface
 * Subclasses must keep viewMat, skyboxViewMat and eye updated
 */
class Camera {
 public:
    virtual ~Camera() {}
    /**
     * Returns the projection matrix
     * For use with shader uniforms or glLoadMatrixf() after calling glMatrixMode(GL_MODELVIEW)
     * @return the modelview matrix as calculated by glm::lookAt(glm::vec3, glm::vec3, glm::vec3)
     */
    glm::mat4 view() const { return viewMat; }
    /**
     * Returns a constant pointer to the cameras modelview matrix
     * This pointer can be used to continuously track the modelview matrix
     * @return A pointer to the modelview matrix
     */
    const glm::mat4 *getViewMatPtr() const { return &viewMat; }
    /**
     * Returns the projection matrix from the perspective required for rendering a skybox (direction only)
     * For use with shader uniforms or glLoadMatrixf() after calling glMatrixMode(GL_MODELVIEW)
     * @return the modelview matrix as calculated by glm::lookAt(glm::vec3, glm::vec3, glm::vec3)
     */
    glm::mat4 skyboxView() const { return skyboxViewMat; }
    /**
     * Returns a constant pointer to the cameras skybox modelview matrix
     * This pointer can be used to continuously track the skybox modelview matrix
     * @return A pointer to the modelview matrix
     */
    const glm::mat4 *getSkyboxViewMatPtr() const { return &skyboxViewMat; }
    /**
     * Returns the cameras location
     * @return The location of the camera in world space
     */
    glm::vec3 getEye() const { return eye; }
    /**
     * Returns a constant pointer to the cameras location
     * This pointer can be used to continuously track the cameras location
     * @return A pointer to the location of the camera in world space
     */
    const glm::vec3 *getEyePtr() const { return &eye; }

 protected:
    explicit Camera(const glm::vec3 eye) : viewMat(0), skyboxViewMat(1), eye(eye) {}
    // View matrix
    glm::mat4 viewMat;
    // View matrix without camera position taken into consideration
    glm::mat4 skyboxViewMat;
    // Eyelocation
    glm::vec3 eye;
};
#endif  // SRC_INTERFACE_CAMERA_H_
