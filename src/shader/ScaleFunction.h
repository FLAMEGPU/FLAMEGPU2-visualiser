#ifndef SRC_SHADER_SCALEFUNCTION_H_
#define SRC_SHADER_SCALEFUNCTION_H_

#include <string>
#include <map>

#include "config/TexBufferConfig.h"

namespace flamegpu {
namespace visualiser {

/**
 * Produces the source for a GLSL function which returns a scale multiplier vector to multiply by each model vertex
 * The function will have the following prototype
 * vec3 getScale()
 */
class ScaleFunction {
 public:
    explicit ScaleFunction(const std::map<TexBufferConfig::Function, TexBufferConfig> &tex_buffers);
    /**
     * Returns the glsl function vec3 getScale()
     */
    std::string getSrc();

 private:
    bool has_scale_x;
    bool has_scale_y;
    bool has_scale_z;
    bool has_uniform_scale;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_SHADER_SCALEFUNCTION_H_
