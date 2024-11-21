#include "flamegpu/visualiser/shader/PositionFunction.h"

#include <map>
#include <string>
#include <sstream>

namespace flamegpu {
namespace visualiser {

    PositionFunction::PositionFunction(const std::map<TexBufferConfig::Function, TexBufferConfig> &tex_buffers)
    : has_pos_x(tex_buffers.find(TexBufferConfig::Position_x) != tex_buffers.end())
    , has_pos_y(tex_buffers.find(TexBufferConfig::Position_y) != tex_buffers.end())
    , has_pos_z(tex_buffers.find(TexBufferConfig::Position_z) != tex_buffers.end())
    , has_pos_xy(tex_buffers.find(TexBufferConfig::Position_xy) != tex_buffers.end())
    , has_pos_xyz(tex_buffers.find(TexBufferConfig::Position_xyz) != tex_buffers.end())
{ }

std::string PositionFunction::getSrc() {
    std::stringstream ss;
    // Define any sampler buffers
    if (has_pos_x) ss << "uniform samplerBuffer _pos_x;" << "\n";
    if (has_pos_y) ss << "uniform samplerBuffer _pos_y;" << "\n";
    if (has_pos_z) ss << "uniform samplerBuffer _pos_z;" << "\n";
    if (has_pos_xy) ss << "uniform samplerBuffer _pos_xy;" << "\n";
    if (has_pos_xyz) ss << "uniform samplerBuffer _pos_xyz;" << "\n";
    // Begin function
    ss << "vec3 getPosition() {" << "\n";
    if (has_pos_x || has_pos_y || has_pos_z) {
        ss << "    return vec3(" <<"\n";
        ss << "        " << (has_pos_x ? "texelFetch(_pos_x, gl_InstanceID).x" : "0") << "," << "\n";
        ss << "        " << (has_pos_y ? "texelFetch(_pos_y, gl_InstanceID).x" : "0") << "," << "\n";
        ss << "        " << (has_pos_z ? "texelFetch(_pos_z, gl_InstanceID).x" : "0") << ");" << "\n";
    } else if (has_pos_xy) {
        ss << "    const int t = gl_InstanceID * 2;" << "\n";
        ss << "    return vec3(" << "\n";
        ss << "        texelFetch(_pos_xy, t).x," << "\n";
        ss << "        texelFetch(_pos_xy, t + 1).x," << "\n";
        ss << "        0);" << "\n";
    } else if (has_pos_xyz) {
        ss << "    const int t = gl_InstanceID * 3;" << "\n";
        ss << "    return vec3(" << "\n";
        ss << "        texelFetch(_pos_xyz, t).x," << "\n";
        ss << "        texelFetch(_pos_xyz, t + 1).x," << "\n";
        ss << "        texelFetch(_pos_xyz, t + 2).x);" << "\n";
    } else {
        ss << "return glm::vec3(0);" << "\n";
    }
    ss << "}" << "\n";

    return ss.str();
}

}  // namespace visualiser
}  // namespace flamegpu
