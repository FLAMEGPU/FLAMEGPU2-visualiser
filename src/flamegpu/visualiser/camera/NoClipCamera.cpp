#include "flamegpu/visualiser/camera/NoClipCamera.h"
#include "flamegpu/visualiser/util/warnings.h"

DISABLE_WARNING_PUSH
#include <glm/gtx/rotate_vector.hpp>
DISABLE_WARNING_POP

#include "flamegpu/visualiser/util/GLcheck.h"

namespace flamegpu {
namespace visualiser {

NoClipCamera::NoClipCamera()
    : NoClipCamera(glm::vec3(1, 1, 1)) {}
NoClipCamera::NoClipCamera(const glm::vec3 &eye)
    : NoClipCamera(eye, glm::vec3(0, 0, 0)) {}
// Initialiser list written to remove any references to member variables
// Because member variables are initialised via initaliser lists in the order they are declared in the class declaration (rather than the order of the initialiser list)
NoClipCamera::NoClipCamera(const glm::vec3 &eye, const glm::vec3 &target)
    : Camera(eye)
    , pureUp(0.0f, 1.0f, 0.0f)
    , look(normalize(target - eye))
    , right(normalize(cross(target - eye, glm::vec3(0, 1, 0))))
    , up(normalize(cross(cross(target - eye, glm::vec3(0, 1, 0)), target - eye)))
    , stabilise(true) {
    // this->eye = eye;                                  // Eye is the location passed by user
    // this->look = target - eye;                        // Look is the direction from eye to target
    // this->right = cross(look, pureUp);                // Right is perpendicular to look and pureUp
    // this->up = cross(right, look);                    // Up is perpendicular to right and look

    this->updateViews();
}
NoClipCamera::~NoClipCamera() {
}
void NoClipCamera::turn(const float &yaw, const float &pitch) {
    // Rotate everything yaw rads about up vector
    this->look = rotate(this->look, -yaw, this->up);
    this->right = rotate(this->right, -yaw, this->up);
    // Rotate everything pitch rads about right vector
    glm::vec3 _look = rotate(this->look, -pitch, this->right);
    glm::vec3 _up = rotate(this->up, -pitch, this->right);
    glm::vec3 _right = this->right;
    if (stabilise) {
        // Right is perpendicular to look and pureUp
        _right = cross(_look, this->pureUp);
        // Stabilised up is perpendicular to right and look
        _up = cross(_right, _look);
        // Don't let look get too close to pure up, else we will spin
        if (std::abs(dot(_look, this->pureUp)) > 0.98)
            return;
    }
    // Commit changes
    this->look = normalize(_look);
    this->right = normalize(_right);
    this->up = normalize(_up);
    this->updateViews();
}
void NoClipCamera::move(const float &distance) {
    eye += look*distance;
    this->updateViews();
}
void NoClipCamera::strafe(const float &distance) {
    eye += right*distance;
    this->updateViews();
}
void NoClipCamera::ascend(const float &distance) {
    eye += pureUp*distance;
    this->updateViews();
}
void NoClipCamera::roll(const float &roll) {
    pureUp = normalize(rotate(pureUp, roll, look));
    right = normalize(rotate(right, roll, look));
    up = normalize(rotate(up, roll, look));
    this->updateViews();
}
void NoClipCamera::setStabilise(const bool &_stabilise) {
    this->stabilise = _stabilise;
}
void NoClipCamera::gluLookAt() {
    GL_CALL(::gluLookAt(
        eye.x, eye.y, eye.z,
        eye.x + look.x, eye.y + look.y, eye.z + look.z,
        up.x, up.y, up.z));
}
void NoClipCamera::skyboxGluLookAt() const {
    GL_CALL(::gluLookAt(
        0, 0, 0,
        look.x, look.y, look.z,
        up.x, up.y, up.z));
}
glm::vec3 NoClipCamera::getLook() const {
    return look;
}
glm::vec3 NoClipCamera::getUp() const {
    return up;
}
glm::vec3 NoClipCamera::getPureUp() const {
    return pureUp;
}
glm::vec3 NoClipCamera::getRight() const {
    return right;
}
void NoClipCamera::updateViews() {
    viewMat = glm::lookAt(eye, eye + look, up);
    skyboxViewMat = glm::lookAt(glm::vec3(0), look, up);
}

}  // namespace visualiser
}  // namespace flamegpu
