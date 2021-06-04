#ifndef SRC_SHADER_DIRECTIONFUNCTION_H_
#define SRC_SHADER_DIRECTIONFUNCTION_H_

#include <string>
#include <map>

#include "config/TexBufferConfig.h"


namespace flamegpu {
namespace visualiser {

/**
 * Produces the source for a GLSL function which returns a rotation matrix to rotate a vector (1,0,0) to match the provided direction
 * The function will have the following prototype
 * mat3 getDirection()
 * The vert.w component of the input value will not be changed
 */
class DirectionFunction {
 public:
    explicit DirectionFunction(const std::map<TexBufferConfig::Function, TexBufferConfig> &tex_buffers);
    /**
     * Returns the glsl function mat3 getDirection()
     */
    std::string getSrc();

 private:
    bool has_fw_x;
    bool has_fw_y;
    bool has_fw_z;
    bool has_up_x;
    bool has_up_y;
    bool has_up_z;
    bool has_heading;
    bool has_pitch;
    bool has_bank;
    static const char* ROTATION_MAT_FN;
    static const char* ATAN2_FN;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_SHADER_DIRECTIONFUNCTION_H_
