#ifndef INCLUDE_CONFIG_STOCK_H_
#define INCLUDE_CONFIG_STOCK_H_

namespace Stock {
namespace Models {
struct Model {
    const char *modelPath;
    const char *texturePath;
};
const Model SPHERE{ "resources/sphere.obj", "" };
const Model ICOSPHERE{ "resources/icosphere.obj", "" };
const Model CUBE{ "resources/cube.obj", "" };
const Model TEAPOT{ "resources/teapot.obj", "" };
}  // namespace Models
}  // namespace Stock

#endif  // INCLUDE_CONFIG_STOCK_H_
