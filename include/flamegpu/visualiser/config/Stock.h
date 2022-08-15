#ifndef INCLUDE_FLAMEGPU_VISUALISER_CONFIG_STOCK_H_
#define INCLUDE_FLAMEGPU_VISUALISER_CONFIG_STOCK_H_

namespace flamegpu {
namespace visualiser {
namespace Stock {
namespace Models {
/**
 * Path to model file and optional texture within integrated resources
 */
struct Model {
    const char *modelPath;
    const char *texturePath;
};
/**
 * Slices and segments sphere
 */
const Model SPHERE{ "resources/sphere.obj", "" };
/*
 * Sphere constructed from triangles
 */
const Model ICOSPHERE{ "resources/icosphere.obj", "" };
/**
 * Cube
 */
const Model CUBE{ "resources/cube.obj", "" };
/**
 * Traditional (sealed) Utah Teapot
 */
const Model TEAPOT{ "resources/teapot.obj", "" };
/**
 * Basic aeroplane model
 * Created following this tutorial: https://www.youtube.com/watch?v=qiK6A4HJhmA
 */
const Model STUNTPLANE{ "resources/stuntplane.obj", "" };
/**
 * Equilateral triangular pyramid (4 poly, 4 vertices)
 *
 * Orientation:
 *   - Front: +X
 *   - Top: +Y
 *
 * Made with blender by zeyus (https://github.com/zeyus)
 */
const Model PYRAMID{ "resources/pyramid.obj", "" };
/**
 * Arrow-head triangular pyramid (4 poly, 4 vertices)
 * useful for visualising vectors
 *
 * Orientation:
 *   - Front: +X
 *   - Top: +Y
 *
 * Made with blender by zeyus (https://github.com/zeyus)
 */
const Model ARROWHEAD{ "resources/arrowhead.obj", "" };
/**
 * Path to model files and optional texture within integrated resources
 */
struct KeyFrameModel {
    const char* modelPathA;
    const char* modelPathB;
    const char* texturePath;
};
/**
 * Two frame pedestrian model
 */
const KeyFrameModel PEDESTRIAN{ "resources/pedestrian_a.obj", "resources/pedestrian_b.obj", "" };
}  // namespace Models
}  // namespace Stock
}  // namespace visualiser
}  // namespace flamegpu

#endif  // INCLUDE_FLAMEGPU_VISUALISER_CONFIG_STOCK_H_
