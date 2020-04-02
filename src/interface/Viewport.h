#ifndef SRC_INTERFACE_VIEWPORT_H_
#define SRC_INTERFACE_VIEWPORT_H_
#include <memory>

#include "multipass/BackBuffer.h"
#include "HUD.h"

class Viewport {
 public:
    /*
     * House keeping incase someone tries to delete a pointer to the ViewPort base class
    */
    virtual ~Viewport() {}
    /**
     * @return The current viewport dimensions
     */
    virtual unsigned int getWindowWidth() const = 0;
    virtual unsigned int getWindowHeight() const = 0;
    virtual glm::uvec2 getWindowDims() const = 0;
    /*
     * Returns the pointer to the view frustum of the viewport
     * It is intended that this value can be tracked by Shaders over the duration of execution
     */
    virtual const glm::mat4 *getProjectionMatPtr() const = 0;
    virtual glm::mat4 getProjectionMat() const = 0;
};
class Camera;
class ViewportExt : public Viewport {
 public:
    virtual ~ViewportExt() {}
    virtual std::shared_ptr<const Camera> getCamera() const = 0;
    virtual std::weak_ptr<HUD> getHUD() = 0;
    /**
    * @return The current window title
    */
    virtual const char *getWindowTitle() const = 0;
    /**
    * Sets the window title
    * @param windowTitle Desired title of the window
    */
    virtual void setWindowTitle(const char *windowTitle) = 0;

 protected:
    void overrideBackBuffer(std::weak_ptr<FBuffer> f) { BackBuffer::setOverride(f); }
    void resetBackBuffer() { BackBuffer::clearOverride(); }
    void resizeBackBuffer(const glm::uvec2 &dims) { BackBuffer::resizeViewport(dims); }
};
#endif  // SRC_INTERFACE_VIEWPORT_H_
