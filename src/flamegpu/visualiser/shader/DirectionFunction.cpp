#include "flamegpu/visualiser/shader/DirectionFunction.h"

#include <sstream>


namespace flamegpu {
namespace visualiser {

const char *DirectionFunction::ROTATION_MAT_FN = R"###(
mat3 RotationMat(vec3 axis, float angle) {
  //Generate rotation matrix
  float cosL = cos(angle);
  float sinL = sin(angle);
  return mat3(
    (cosL + (axis.x*axis.x*(1-cosL))),     ((axis.x*axis.y*(1-cosL))-(axis.z*sinL)), ((axis.x*axis.z*(1-cosL))+(axis.y*sinL)),
    ((axis.y*axis.x*(1-cosL))+(axis.z*sinL)), (cosL + (axis.y*axis.y*(1-cosL))),     ((axis.y*axis.z*(1-cosL))-(axis.x*sinL)),
    ((axis.z*axis.x*(1-cosL))-(axis.y*sinL)), ((axis.z*axis.y*(1-cosL))+(axis.x*sinL)), (cosL + (axis.z*axis.z*(1-cosL)))
  );
}
)###";
/**
 * Based on: https://stackoverflow.com/questions/26070410/robust-atany-x-on-glsl-for-converting-xy-coordinate-to-angle
 * Testing showed this returns something other than 0 when atan2(0,0) is called on my hardware (atan(0,0) is undefined in GLSL)
 * Which would provide an unwated rotation for the case target=(0,1,0) or target=(0,-1,0)
 */
const char* DirectionFunction::ATAN2_FN = R"###(
float atan2(float y, float x) {
  if (y==0 && x==0)
    return 0;
  bool s = (abs(x) > abs(y));
  return mix(3.1415926538/2.0 - atan(x,y), atan(y,x), s);
}
)###";


DirectionFunction::DirectionFunction(const std::map<TexBufferConfig::Function, TexBufferConfig> &tex_buffers)
    : has_fw_x(tex_buffers.find(TexBufferConfig::Forward_x) != tex_buffers.end())
    , has_fw_y(tex_buffers.find(TexBufferConfig::Forward_y) != tex_buffers.end())
    , has_fw_z(tex_buffers.find(TexBufferConfig::Forward_z) != tex_buffers.end())
    , has_up_x(tex_buffers.find(TexBufferConfig::Up_x) != tex_buffers.end())
    , has_up_y(tex_buffers.find(TexBufferConfig::Up_y) != tex_buffers.end())
    , has_up_z(tex_buffers.find(TexBufferConfig::Up_z) != tex_buffers.end())
    , has_heading(tex_buffers.find(TexBufferConfig::Heading) != tex_buffers.end())
    , has_pitch(tex_buffers.find(TexBufferConfig::Pitch) != tex_buffers.end())
    , has_bank(tex_buffers.find(TexBufferConfig::Bank) != tex_buffers.end())
{ }

std::string DirectionFunction::getSrc() {
    /**
     * Optional texture buffers
     * _fw_x, _fw_y, _fw_z: Heading direction vector
     * _up_x, _up_y, _up_z: Heading up vector
     * _heading: Angle to rotate about y axis
     * _pitch: Angle to rotate about x axis
     * _bank: Angle to rotate about z axis
     *
     * Heading requires: _heading or [_fw_x, _fw_z]
     * Pitch requires: _pitch or _fw_y
     * Bank requires: _bank or [_fw_x, _fw_z, _up_x, _up_y, _up_z]
     */
    std::stringstream ss;
    // First define the two utility functions
    ss << ROTATION_MAT_FN;
    ss << ATAN2_FN;
    // Define any sampler buffers
    if (has_fw_x) ss << "uniform samplerBuffer _fw_x;" << "\n";
    if (has_fw_y) ss << "uniform samplerBuffer _fw_y;" << "\n";
    if (has_fw_z) ss << "uniform samplerBuffer _fw_z;" << "\n";
    if (has_up_x) ss << "uniform samplerBuffer _up_x;" << "\n";
    if (has_up_y) ss << "uniform samplerBuffer _up_y;" << "\n";
    if (has_up_z) ss << "uniform samplerBuffer _up_z;" << "\n";
    if (has_heading) ss << "uniform samplerBuffer _heading;" << "\n";
    if (has_pitch) ss << "uniform samplerBuffer _pitch;" << "\n";
    if (has_bank) ss << "uniform samplerBuffer _bank;" << "\n";
    // Begin function
    ss << "mat3 getDirection() {" << "\n";
    // Define vectors for our global coordinate system
    ss << "vec3 FORWARD = vec3(1, 0, 0); // Aka the initial direction" << "\n";
    ss << "vec3 UP = vec3(0, 1, 0);" << "\n";
    ss << "vec3 RIGHT = vec3(0, 0, 1);" << "\n";
    // Grab model direction from texture array
    if (has_fw_x || has_fw_y || has_fw_z) {
        // missing buffers always return 0
        ss << "vec3 target = vec3(" << "\n";
        ss << (has_fw_x ? "    texelFetch(_fw_x, gl_InstanceID).x" : "0") << "," << "\n";
        ss << (has_fw_y ? "    texelFetch(_fw_y, gl_InstanceID).x" : "0") << "," << "\n";
        ss << (has_fw_z ? "    texelFetch(_fw_z, gl_InstanceID).x" : "0") << ");" << "\n";
        // If target is null, don't rotate
        ss << "if (target.xyz == vec3(0))" << "\n";
        ss << "    return mat3(1);" << "\n";
        // Normalize target incase the user forgot
        ss << "target = normalize(target);" << "\n";
    } else {
        if (has_heading) {
            ss << "const float angle_H = texelFetch(_heading, gl_InstanceID).x" << "\n";
        }
        if (has_pitch) {
            ss << "const float angle_P = texelFetch(_pitch, gl_InstanceID).x" << "\n";
        }
    }
    if (has_fw_x && has_fw_y && has_fw_z && has_up_x && has_up_y && has_up_z) {
        ss << "vec3 target_up = vec3(" << "\n";
        ss << "    texelFetch(_up_x, gl_InstanceID).x," << "\n";
        ss << "    texelFetch(_up_y, gl_InstanceID).x," << "\n";
        ss << "    texelFetch(_up_z, gl_InstanceID).x);" << "\n";
    } else if (has_bank) {
        ss << "const float angle_B = texelFetch(_bank, gl_InstanceID).x" << "\n";
    }
    // Begin to perform rotation
    ss << "mat3 rm = mat3(1);" << "\n";
    // Euler angle extraction is based on: https://stackoverflow.com/questions/21622956/how-to-convert-direction-vector-to-euler-angles
     // Apply rotation about 1st axis
    if (has_heading || (has_fw_x || has_fw_z)) {
        // Calculate the heading angle (yaw, about Y)
        if (!has_heading) {
            ss << "float angle_H = atan2(target.z, target.x);" << "\n";
        }
        // Apply the heading angle
        ss << "rm = RotationMat(UP, angle_H) * rm;" << "\n";
    }
    // Apply rotation about 2nd axis
    if (has_pitch || has_fw_y) {
        // Calculate the pitch angle (pitch, about Z)
        if (!has_pitch) {
            ss << "float angle_P = -asin(target.y);" << "\n";
        }
        // Apply the pitch angle
        ss << "rm = RotationMat(rm * RIGHT, angle_P) * rm;" << "\n";
    }
    // Apply rotation about 3rd axis
    if (has_bank || (has_fw_x && has_fw_z && has_up_x && has_up_y && has_up_z)) {
        // Calculate the bank angle (roll, about X)
        if (!has_bank) {
            ss << "float angle_B = 0; " << "\n";
            ss << "if (target.x != 0 || target.z != 0) {" << "\n";
            ss << "    vec3 W0 = vec3(-target.z, 0, target.x);" << "\n";
            ss << "    vec3 U0 = cross(W0, target);" << "\n";
            ss << "    angle_B = atan2(dot(W0, target_up) / length(W0), dot(U0, target_up) / length(U0));" << "\n";
            ss << "} else {" << "\n";
            // The provided algorithm breaks for target==(0,1,0) or target==(0,-1,0)
            // The above branch, sets W0 0, which causes cross product to fail etc etc
            // This fix applies the angle_H technique to UP_2 to extract an appropriate angle
            ss << "    angle_B = atan2(target_up.z, target_up.x);" << "\n";
            ss << "    angle_B = target.y > 0 ? 3.1415926538 + angle_B : angle_B;" << "\n";
            // This alternate version was used before I fixed atan2(0,0) to return 0
            // I also believe which was erroneously causing angle_H to rotate 90 degrees
            // ss << "    angle_B = atan2(target_up.x, target_up.z);" << "\n";
            // ss << "    angle_B = target.y > 0 ? 3.1415926538 - angle_B - angle_H : angle_H - angle_B;" << "\n";
            ss << "}" << "\n";
        }
        // Apply the bank angle
        ss << "rm = RotationMat(rm * FORWARD, angle_B) * rm;" << "\n";
    }
    ss << "    return rm;" << "\n";
    ss << "}" << "\n";

    return ss.str();
}

}  // namespace visualiser
}  // namespace flamegpu
