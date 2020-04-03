#include "Overlay.h"

#include "util/warnings.h"
DISABLE_WARNING_PUSH
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNING_POP

#include "shader/Shaders.h"

void Overlay::setWidth(unsigned int w) {
    setDimensions({ w, dimensions.y });
}
void Overlay::setHeight(unsigned int h) {
    setDimensions({ dimensions.x, h });
}
void Overlay::setDimensions(unsigned int w, unsigned int h) {
    setDimensions({ w, h });
}
void Overlay::setDimensions(glm::uvec2 dims) {
    if (dimensions == dims)
        return;
    dimensions = dims;
    if (auto t = hudItem.lock())
        t->resizeWindow();
}
void Overlay::setHUDItem(std::shared_ptr<HUD::Item> ptr) {
    hudItem = std::weak_ptr<HUD::Item>(ptr);
}
Overlay::Overlay(std::shared_ptr<Shaders> shaders, unsigned int width, unsigned int height)
    :Overlay(shaders, {width, height}) {
}
Overlay::Overlay(std::shared_ptr<Shaders> shaders, glm::uvec2 dimensions)
    : hudItem()
    , visible(true)
    , shaders(shaders)
    , dimensions(dimensions) {
}
void Overlay::render(const glm::mat4 *mv, const glm::mat4 *proj, GLuint fbo) {
    if (!visible)
        return;
    if (this->shaders != nullptr) {
        shaders->setViewMatPtr(mv);
        shaders->setProjectionMatPtr(proj);
        shaders->useProgram();
    } else {  // Lets try fixed function, but no promises
        // Update Projection Matrix
        glMatrixMode(GL_PROJECTION);
        GL_CALL(glLoadMatrixf(glm::value_ptr(*proj)));
        // Update Model View Matrix
        glMatrixMode(GL_MODELVIEW);
        GL_CALL(glLoadMatrixf(glm::value_ptr(*mv)));
    }
    // Bind the faces to be rendered
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbo));
    GL_CALL(glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0));
    // GL_CALL(glDisable(GL_BLEND));
    if (this->shaders != nullptr)
        shaders->clearProgram();
    // Do something, setup projection/quad
}
void Overlay::setVisible(bool isVisible) {
    this->visible = isVisible;
}
void Overlay::_reload() {
    shaders->reload(); reload();
}
