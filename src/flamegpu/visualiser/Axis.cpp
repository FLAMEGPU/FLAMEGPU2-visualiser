#include "Axis.h"

namespace flamegpu {
namespace visualiser {

/*
Constructs a new axis object
@param length The length of each drawn axis
*/
Axis::Axis(float length)
    : pen(6) {
        pen.begin(Draw::Type::Lines, "axis");
        {
            // x
            pen.color(1, 0, 0);
            pen.vertex(0, 0, 0);
            pen.vertex(length, 0, 0);
            // y
            pen.color(0, 1, 0);
            pen.vertex(0, 0, 0);
            pen.vertex(0, length, 0);
            // z
            pen.color(0, 0, 1);
            pen.vertex(0, 0, 0);
            pen.vertex(0, 0, length);
        }
        pen.save();
}
void Axis::render() {
    pen.render("axis");
}
void Axis::reload() {
    pen.reload();
}
void Axis::setViewMatPtr(glm::mat4 const *viewMat) {
    pen.setViewMatPtr(viewMat);
}
void Axis::setProjectionMatPtr(glm::mat4 const *projectionMat) {
    pen.setProjectionMatPtr(projectionMat);
}
void Axis::setLightsBuffer(const GLuint &bufferBindingPoint) {
    pen.setLightsBuffer(bufferBindingPoint);
}

}  // namespace visualiser
}  // namespace flamegpu
