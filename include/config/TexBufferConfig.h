#ifndef INCLUDE_CONFIG_TEXBUFFERCONFIG_H_
#define INCLUDE_CONFIG_TEXBUFFERCONFIG_H_

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
        /**
         * Agent forward/up direction x/y/z
         */
        Forward_x, Forward_y, Forward_z,
        Up_x, Up_y, Up_z,
        /**
         * Agent rotation
         * (Alternate to Forward/Up vectors)
         */
        Heading, Pitch, Bank,
        /**
         * Agent color
         */
        Color,
        /**
         * Agent scale
         */
        Scale_x, Scale_y, Scale_z,
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
        case Position_x: return "x_pos";
        case Position_y: return "y_pos";
        case Position_z: return "z_pos";
        case Forward_x: return "_fw_x";
        case Forward_y: return "_fw_y";
        case Forward_z: return "_fw_z";
        case Up_x: return "_up_x";
        case Up_y: return "_up_y";
        case Up_z: return "_up_z";
        case Heading: return "_heading";
        case Pitch: return "_pitch";
        case Bank: return "_bank";
        case Scale_x: return "_scale_x";
        case Scale_y: return "_scale_y";
        case Scale_z: return "_scale_z";
        case UniformScale: return "_scale";
        // These always get name from elsewhere so return empty string
        case Color:
        case Unknown:
        default: return "";
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
    CustomTexBufferConfig(const std::string& _agentVariableName, const std::string& _nameInShader)
        : TexBufferConfig(_agentVariableName)
        , nameInShader(_nameInShader) { }
    /**
     * Name in shader, this may be ignored by some functions
     */
    std::string nameInShader;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // INCLUDE_CONFIG_TEXBUFFERCONFIG_H_
