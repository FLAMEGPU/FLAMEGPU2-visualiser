#ifndef SRC_FLAMEGPU_VISUALISER_AXIS_H_
#define SRC_FLAMEGPU_VISUALISER_AXIS_H_
#include "flamegpu/visualiser/interface/Renderable.h"
#include "Draw.h"

namespace flamegpu {
namespace visualiser {

class Axis : public Renderable {
 public:
    /**
     * Allocates buffer objects for the vertices/colors/face-indices of axis marker of the given length
     * @param length The length of each axis marker line
     */
    explicit Axis(float length = 1.0);
    /**
     * Renders a simple axis marker. Red displays the positive x, Green the positive y and Blue the positive z.
     */
    void render();
    /**
     * Reloads the shader
     */
    void reload() override;
    /**
     * Provides view matrix to the shader
     */
    void setViewMatPtr(glm::mat4 const *viewMat) override;
    /**
    * Provides projection matrix to the shader
    */
    void setProjectionMatPtr(glm::mat4 const *projectionMat) override;
    /**
    * Provides lights buffer to the shader
    * @param bufferBindingPoint Set the buffer binding point to be used for rendering
    */
    void setLightsBuffer(const GLuint &bufferBindingPoint) override;

 private:
    Draw pen;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_AXIS_H_
