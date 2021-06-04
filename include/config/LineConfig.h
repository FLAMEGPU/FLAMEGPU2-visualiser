#ifndef INCLUDE_CONFIG_LINECONFIG_H_
#define INCLUDE_CONFIG_LINECONFIG_H_
#include <vector>

namespace flamegpu {
namespace visualiser {

/**
 * Represents a user defined line drawing
 */
struct LineConfig {
    enum class Type {Lines, Polyline};
    explicit LineConfig(Type t) : lineType(t) { }
    /**
     * Whether line is one continuous line or many separate lines
     * If type == Lines, length of vertices must be a multiple of 6
     * If type == Polyline, length of vertices must be a multiple of 3 (and greater than 3)
     */
    const Type lineType;
    /**
     * Vector filled with 3 floats per vertex
     *
     */
    std::vector<float> vertices;
    /**
     * Vector filled with 4 floats per vertex (must be same 4/3 the length of vertices vector)
     */
    std::vector<float> colors;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // INCLUDE_CONFIG_LINECONFIG_H_
