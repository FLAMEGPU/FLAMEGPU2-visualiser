#ifndef SRC_SPRITE2D_H_
#define SRC_SPRITE2D_H_
#include <memory>

#include "Texture/Texture2D.h"
#include "Overlay.h"

class Shaders;
class Texture2D;
/**
 * Class for rendering 2D graphics to the screen via an orthographic projection
 */
class Sprite2D : public Overlay {
 public:
//    /**
//     * Creates a new 2D Sprite overlay from the given image
//     * @param imagePath Path to the image to be used
//     * @param shader The shader to be used, if not provided will render as though standard RGBA image
//     * @param dimensions The dimensions of the overlay, if 0 is passed, the image height will be used unless width was specified, in which case the image will be scaled, preserving aspect ratio
//     */
//   Sprite2D(const char *imagePath, std::shared_ptr<Shaders> shader = nullptr, glm::uvec2 dimensions = glm::uvec2(0));
    /**
     * Creates a new 2D Sprite overlay from the provide GL_TEXTURE_2D
     * @param texName The GL texture name as provided by glGenTextures()
     * @param dimensions The dimensions of the overlay
     * @param shader The shader to be used, if not provided will render as though standard RGBA image
     */
    Sprite2D(GLuint texName, GLuint texUnit, glm::uvec2 dimensions = glm::uvec2(0), std::shared_ptr<Shaders> shader = nullptr);
    /**
     * Creates a new 2D Sprite overlay from the provide GL_TEXTURE_2D
     * @param tex The texture to display
     * @param shader The shader to be used, if not provided will render as though standard RGBA image
     * @param dimensions The dimensions of the overlay
     */
    explicit Sprite2D(std::shared_ptr<const Texture2D> tex, std::shared_ptr<Shaders> shader = nullptr, glm::uvec2 dimensions = glm::uvec2(0));
    virtual ~Sprite2D() {}
    void reload() override {}

 private:
    std::shared_ptr<const Texture2D> tex;
};
#endif  // SRC_SPRITE2D_H_
