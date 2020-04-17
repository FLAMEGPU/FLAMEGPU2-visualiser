#ifndef INCLUDE_CONFIG_STOCK_H_
#define INCLUDE_CONFIG_STOCK_H_

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
}  // namespace Models
}  // namespace Stock

#endif  // INCLUDE_CONFIG_STOCK_H_
