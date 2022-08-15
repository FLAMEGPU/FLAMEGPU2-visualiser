#ifndef SRC_FLAMEGPU_VISUALISER_SHADER_VERTEXFUNCTION_H_
#define SRC_FLAMEGPU_VISUALISER_SHADER_VERTEXFUNCTION_H_

#include <string>
#include <map>

#include "flamegpu/visualiser/config/TexBufferConfig.h"


namespace flamegpu {
namespace visualiser {

/**
 * Produces the source for a GLSL function which returns a vertex of the model
 * The function will have the following prototype
 * vec3 getVertex()
 */
class VertexFunction {
 public:
    explicit VertexFunction(const std::map<TexBufferConfig::Function, TexBufferConfig> &tex_buffers, const char *modelpathB);
    /**
     * Returns the glsl function vec3 getVertex()
     */
    std::string getSrc() const;

 private:
    bool has_animation_lerp;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_SHADER_VERTEXFUNCTION_H_
