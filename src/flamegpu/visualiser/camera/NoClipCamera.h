#ifndef SRC_FLAMEGPU_VISUALISER_CAMERA_NOCLIPCAMERA_H_
#define SRC_FLAMEGPU_VISUALISER_CAMERA_NOCLIPCAMERA_H_
#include <utility>

#include "flamegpu/visualiser/interface/Camera.h"

namespace flamegpu {
namespace visualiser {

class NoClipCamera : public Camera {
 public:
    /**
     * Initialises the camera located at (1, 1, 1) directed at (0, 0, 0)
     */
    NoClipCamera();
    /**
     * Initialises the camera located at eye, directed at (0, 0, 0)
     * @param eye The coordinates the camera is located
     */
    explicit NoClipCamera(const glm::vec3 &eye);
    /**
    * Initialises the camera located at eye directed at target
    * @param eye The coordinates the camera is located
    * @param target The coordinates the camera is directed towards
    * @param _roll The camera roll in radians
    */
    NoClipCamera(const glm::vec3 &eye, const glm::vec3 &target, float _roll = 0);
    /**
     * Default destructor
     */
    ~NoClipCamera();

    /**
     * Rotate look and right, yaw radians about up
     * Rotate look and up, pitch radians about right
     * @param yaw The number of radians to rotate the camera's direction about the up vector
     * @param pitch The number of radians to rotate the camera's direction about the right vector
     */
    void turn(const float &yaw, const float &pitch);
    /**
     * Move eye specified distance along look
     * @param distance The number of units to move the camera
     */
    void move(const float &distance);
    /**
     * Move eye specified distance along right
     * @param distance The number of units to move the camera
     */
    void strafe(const float &distance);
    /**
     * Move eye specified distance along pureUp
     * @param distance The number of units to move the camera
     */
    void ascend(const float &distance);
    /**
     * Rotate right and up, roll radians about look
     * @param roll The number of radians to rotate the camera's direction about the look vector
     */
    void roll(const float &roll);
    /**
     * Sets whether the camera should be stabilised
     * When the camera is stabilised, the up vector will not rotate about the cameras direction
     * When the camera is not stabilsed, moving the mouse in a circular motion may cause the camera to roll
     * @param stabilise Whether the camera should be stabilised
     */
    void setStabilise(const bool &stabilise);
    /**
     * Returns the cameras normalized direction vector
     * @return The normalized direction of the camera
     */
    glm::vec3 getLook() const;
    /**
     * Returns the cameras normalized up vector
     * @return The normalized direction the camera treats as upwards
     */
    glm::vec3 getUp() const;
    /**
     * Returns the value of pureUp
     * This value is used by the stabilisation to prevent the camera rolling unintentionally
     * @return The normalized direction the camera treats as the true up
     */
    glm::vec3 getPureUp() const;
    /**
     * Returns the cameras normalized right vector
     * @return The normalized direction the camera treats as rightwards
     */
    glm::vec3 getRight() const;

 private:
    /**
     * Updates the view and skyboxView matrices
     * Called whenever any internal camera variables are updated
     */
    void updateViews();
    // Up vector used for stabilisation, only rotated when roll is called
    glm::vec3 pureUp;
    // 3 perpendicular vectors which represent the cameras direction and orientation
    glm::vec3 look;
    glm::vec3 right;
    glm::vec3 up;
    bool stabilise;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_CAMERA_NOCLIPCAMERA_H_
