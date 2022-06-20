#ifndef INCLUDE_FLAMEGPU_VISUALISER_CONFIG_TEXBUFFERCONFIG_H_
#define INCLUDE_FLAMEGPU_VISUALISER_CONFIG_TEXBUFFERCONFIG_H_

#include <string>
#include <typeindex>

namespace flamegpu {
namespace visualiser {

/**
 * Holds data identifying a requested texture buffer
 */
struct TexBufferConfig {
    enum Function {
        /**
         * Agent location x/y/z
         */
        Position_x, Position_y, Position_z,
        Position_xy,
        Position_xyz,
        /**
         * Agent forward/up direction x/y/z
         */
        Forward_x, Forward_y, Forward_z,
        Forward_xz,
        Forward_xyz,
        Up_x, Up_y, Up_z,
        Up_xyz,
        /**
         * Agent rotation
         * (Alternate to Forward/Up vectors)
         */
        Heading, Pitch, Bank,
        Direction_hp,
        Direction_hpb,
        /**
         * Agent color
         */
        Color,
        /**
         * Agent scale
         */
        Scale_x, Scale_y, Scale_z,
        Scale_xy,
        Scale_xyz,
        /**
         * Uniform agent scale
         * (Alternate to individual scale components)
         */
        UniformScale,
        /**
         * Some other use, e.g. custom shaders in future
         */
        Unknown,
    };
    static std::string SamplerName(Function f) {
        switch (f) {
        case Position_x: return "_pos_x";
        case Position_y: return "_pos_y";
        case Position_z: return "_pos_z";
        case Position_xy: return "_pos_xy";
        case Position_xyz: return "_pos_xyz";
        case Forward_x: return "_fw_x";
        case Forward_y: return "_fw_y";
        case Forward_z: return "_fw_z";
        case Forward_xz: return "_fw_xz";
        case Forward_xyz: return "_fw_xyz";
        case Up_x: return "_up_x";
        case Up_y: return "_up_y";
        case Up_z: return "_up_z";
        case Up_xyz: return "_up_xyz";
        case Heading: return "_heading";
        case Pitch: return "_pitch";
        case Bank: return "_bank";
        case Direction_hp: return "_direction_hp";
        case Direction_hpb: return "_direction_hpb";
        case Scale_x: return "_scale_x";
        case Scale_y: return "_scale_y";
        case Scale_z: return "_scale_z";
        case Scale_xy: return "_scale_x";
        case Scale_xyz: return "_scale_xyz";
        case UniformScale: return "_scale";
        // These always get name from elsewhere so return empty string
        case Color:
        case Unknown:
        default: return "";
        }
    }
    static int SamplerElements(Function f) {
        switch (f) {
        case Position_xyz:
        case Forward_xyz:
        case Up_xyz:
        case Direction_hpb:
        case Scale_xyz:
            return 3;
        case Position_xy:
        case Forward_xz:
        case Direction_hp:
        case Scale_xy:
            return 2;
        case Position_x:
        case Position_y:
        case Position_z:
        case Forward_x:
        case Forward_y:
        case Forward_z:
        case Up_x:
        case Up_y:
        case Up_z:
        case Heading:
        case Pitch:
        case Bank:
        case Scale_x:
        case Scale_y:
        case Scale_z:
        case UniformScale:
        case Color:
        case Unknown:
        default:
            return 1;
        }
    }
    explicit TexBufferConfig(const std::string& _agentVariableName = "")
        : agentVariableName(_agentVariableName) { }
    /**
     * Agent variable which maps to this buffer
     */
    std::string agentVariableName;
    /**
     * This value is updated when passing the map to the update texture buffer function
     */
    void *t_d_ptr = nullptr;
};

struct CustomTexBufferConfig : TexBufferConfig {
    CustomTexBufferConfig(const std::string& _agentVariableName, const std::string& _nameInShader, const unsigned int _element, const unsigned int _array_length)
        : TexBufferConfig(_agentVariableName)
        , nameInShader(_nameInShader)
        , element(_element)
        , array_length(_array_length) { }
    /**
     * Name in shader, this may be ignored by some functions
     */
    std::string nameInShader;
    /**
     * Index of the element to be used if an array variable (else 0)
     */
    unsigned int element;
    /**
     * Length of the variable if an array variable (else 1)
     */
    unsigned int array_length;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // INCLUDE_FLAMEGPU_VISUALISER_CONFIG_TEXBUFFERCONFIG_H_
