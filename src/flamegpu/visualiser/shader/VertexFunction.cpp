#include "flamegpu/visualiser/shader/VertexFunction.h"

#include <sstream>


namespace flamegpu {
namespace visualiser {

VertexFunction::VertexFunction(const std::map<TexBufferConfig::Function, TexBufferConfig> &tex_buffers, const char *modelpathB)
    : has_animation_lerp(tex_buffers.find(TexBufferConfig::AnimationLerp) != tex_buffers.end() && modelpathB)
{ }

std::string VertexFunction::getSrc() const {
    std::stringstream ss;
    // in vec3 _vertex; is predefined in the template
    // Define any additional inputs buffers
    if (has_animation_lerp) {
        ss << "in vec3 _vertex2;" << "\n";
        ss << "in vec3 _normal2;" << "\n";
        ss << "uniform samplerBuffer _animation_lerp;" << "\n";
    }
    // Begin vertex function
    ss << "vec3 getVertex() {" << "\n";
    if (has_animation_lerp) {
        ss << "    const float lerp = texelFetch(_animation_lerp, gl_InstanceID).x;" << "\n";
        ss << "    return mix(_vertex, _vertex2, lerp);" << "\n";
    } else {
        ss << "    return _vertex;" << "\n";
    }
    ss << "}" << "\n";
    // Begin normal function
    ss << "vec3 getNormal() {" << "\n";
    if (has_animation_lerp) {
        ss << "    const float lerp = texelFetch(_animation_lerp, gl_InstanceID).x;" << "\n";
        ss << "    return mix(normalize(_normal), normalize(_normal2), lerp);" << "  // assumes the caller will normalize the return\n";
    } else {
        ss << "    return _normal;" << "\n";
    }
    ss << "}" << "\n";

    return ss.str();
}

}  // namespace visualiser
}  // namespace flamegpu
