#include "shader/ScaleFunction.h"

#include <sstream>

namespace flamegpu {
namespace visualiser {

ScaleFunction::ScaleFunction(const std::map<TexBufferConfig::Function, TexBufferConfig> &tex_buffers)
    : has_scale_x(tex_buffers.find(TexBufferConfig::Scale_x) != tex_buffers.end())
    , has_scale_y(tex_buffers.find(TexBufferConfig::Scale_y) != tex_buffers.end())
    , has_scale_z(tex_buffers.find(TexBufferConfig::Scale_z) != tex_buffers.end())
    , has_uniform_scale(tex_buffers.find(TexBufferConfig::UniformScale) != tex_buffers.end())
{ }

std::string ScaleFunction::getSrc() {
    /**
     * Optional texture buffers
     * scale_x, scale_y, scale_z: Individual component scale multipliers
     * or
     * _scale: Uniform scale component
     */
    std::stringstream ss;
    // Define any sampler buffers
    if (has_scale_x) ss << "uniform samplerBuffer _scale_x;" << "\n";
    if (has_scale_y) ss << "uniform samplerBuffer _scale_y;" << "\n";
    if (has_scale_z) ss << "uniform samplerBuffer _scale_z;" << "\n";
    if (has_uniform_scale) ss << "uniform samplerBuffer _scale;" << "\n";
    // Begin function
    ss << "vec3 getScale() {" << "\n";
    // Grab model scale from texture array
    if (has_scale_x || has_scale_y || has_scale_z) {
        // missing buffers always return 1.0
        ss << "    " << "return vec3(" << "\n";
        ss << "        " << (has_scale_x ? "texelFetch(_scale_x, gl_InstanceID).x" : "1.0") << "," << "\n";
        ss << "        " << (has_scale_y ? "texelFetch(_scale_y, gl_InstanceID).x" : "1.0") << "," << "\n";
        ss << "        " << (has_scale_z ? "texelFetch(_scale_z, gl_InstanceID).x" : "1.0") << ");" << "\n";
    } else if (has_uniform_scale) {
        ss << "    " << "return vec3(texelFetch(_scale, gl_InstanceID).x);" << "\n";
    } else {
        ss << "    " << "return vec3(1.0);" << "\n";
    }
    ss << "}" << "\n";

    return ss.str();
}

}  // namespace visualiser
}  // namespace flamegpu
