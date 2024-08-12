#ifndef SRC_FLAMEGPU_VISUALISER_SHADER_POSITIONFUNCTION_H_
#define SRC_FLAMEGPU_VISUALISER_SHADER_POSITIONFUNCTION_H_

#include <string>
#include <map>

#include "flamegpu/visualiser/config/TexBufferConfig.h"


namespace flamegpu {
namespace visualiser {

/**
 * Produces the source for a GLSL function which returns a position vector
 * The function will have the following prototype
 * vec3 getPosition()
 */
class PositionFunction {
 public:
    explicit PositionFunction(const std::map<TexBufferConfig::Function, TexBufferConfig> &tex_buffers);
    /**
     * Returns the glsl function mat3 getPosition()
     */
    std::string getSrc();

 private:
    bool has_pos_x;
    bool has_pos_y;
    bool has_pos_z;
    bool has_pos_xy;
    bool has_pos_xyz;
    bool has_pos_dbl_x;
    bool has_pos_dbl_y;
    bool has_pos_dbl_z;
    bool has_pos_dbl_xy;
    bool has_pos_dbl_xyz;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_SHADER_POSITIONFUNCTION_H_
