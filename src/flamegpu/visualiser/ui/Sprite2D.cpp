#include "Sprite2D.h"
#include "flamegpu/visualiser/shader/Shaders.h"

namespace flamegpu {
namespace visualiser {


// Sprite2D::Sprite2D(const char *imagePath, std::shared_ptr<Shaders> shader, glm::uvec2 dimensions)
//     : Sprite2D(Texture2D::load(imagePath), shader, dimensions)
// { }
Sprite2D::Sprite2D(GLuint texName, GLuint texUnit, glm::uvec2 dimensions, std::shared_ptr<Shaders> shader)
    : Overlay(shader == nullptr ? std::make_shared<Shaders>(Stock::Shaders::SPRITE2D) : shader)
    , tex(nullptr) {
    setDimensions(dimensions);
    getShaders()->addTexture("_texture", GL_TEXTURE_2D, texName, texUnit);
}

Sprite2D::Sprite2D(std::shared_ptr<const Texture2D> tex, std::shared_ptr<Shaders> shader, glm::uvec2 dimensions)
    : Overlay(shader == nullptr ? std::make_shared<Shaders>(Stock::Shaders::SPRITE2D) : shader)
    , tex(tex) {
    unsigned int width = dimensions.x == 0 ? tex->getWidth() : dimensions.x;
    unsigned int height = dimensions.y == 0 ? (dimensions.x == 0 ? tex->getHeight() : (tex->getHeight()*width / tex->getWidth())) : dimensions.y;
    setDimensions({ width, height });
    getShaders()->addTexture("_texture", tex);
}

}  // namespace visualiser
}  // namespace flamegpu
