#ifndef SRC_OVERLAY_H_
#define SRC_OVERLAY_H_
#include <memory>

#include "texture/Texture2D.h"
#include "HUD.h"

class Shaders;

/*
Represents a 2d quad rendered in orthographic over the screen.
*/
class Overlay {
    friend class HUD;

 public:
    /**
     * Creates a new overlay, this is an abstract class and should not be directly instantiated
     * @param shaders Shared pointer to the shaders object to be used when rendering the overlay
     * @param width The width of the overlay
     * @param height The height of the overlay
     * @note If the dimensions of the overlay are not known at initialisation, call setDimensions() as soon as they are known.
     */
    Overlay(std::shared_ptr<Shaders> shaders, unsigned int width, unsigned int height);
    /**
     * Creates a new overlay, this is an abstract class and should not be directly instantiated
     * @param shaders Shared pointer to the shaders object to be used when rendering the overlay
     * @param dimensions The dimensions of the overlay
     * @note If the dimensions of the overlay are not known at initialisation, call setDimensions() as soon as they are known.
     */
    explicit Overlay(std::shared_ptr<Shaders> shaders, glm::uvec2 dimensions = glm::uvec2(0));
    virtual ~Overlay() {}
    void _reload();
    virtual void reload() = 0;
    /**
     * Renders the overlay using the provided details
     * @param mv The modelview matrix
     * @param proj The projection matrix
     * @param fbo The buffer object holding the face indices
     */
    void render(const glm::mat4 *mv, const glm::mat4 *proj, GLuint fbo);
    unsigned int getWidth() const { return dimensions.x; }
    unsigned int getHeight() const { return dimensions.y;}
    std::shared_ptr<Shaders> getShaders() const { return shaders; }

    /**
     * Sets whether the overlay should be rendered or not
     * @param isVisible True if the overlay should be rendered
     */
    void setVisible(bool isVisible);
    bool getVisible() const { return visible; }

 protected:
    /**
     * Updates the overlays width and triggers HUD::Item.resizeWindow() if available.
     * @param w The new overlay width
     */
    void setWidth(unsigned int w);
    /**
     * Updates the overlays height and triggers HUD::Item.resizeWindow() if available.
     * @param h The new overlay height
     */
    void setHeight(unsigned int h);
    /**
     * Updates the overlays dimensions and triggers HUD::Item.resizeWindow() if available.
     * @param w The new overlay width
     * @param h The new overlay height
     */
    void setDimensions(unsigned int w, unsigned int h);
    /**
    * Updates the overlays dimensions and triggers HUD::Item.resizeWindow() if available.
    * @param dims The new overlay dimensions
    */
    void setDimensions(glm::uvec2 dims);
    /**
     * Pointer to the owning HUD item
     */
    std::weak_ptr<HUD::Item> hudItem;

 private:
    bool visible;
    /**
     * Sets the HUD::item attatched to the overlay, so that resize events can be triggered
     */
    void setHUDItem(std::shared_ptr<HUD::Item> ptr);
    std::shared_ptr<Shaders> shaders;
    glm::uvec2 dimensions;
};

#endif  // SRC_OVERLAY_H_
