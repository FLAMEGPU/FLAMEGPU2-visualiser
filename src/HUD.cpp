#include "HUD.h"

#include "util/warnings.h"

DISABLE_WARNING_PUSH
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNING_POP

#include "Overlay.h"
#include "shader/Shaders.h"


HUD::HUD(const unsigned int &width, const unsigned int &height)
    : HUD(glm::uvec2(width, height)) { }

HUD::HUD(const glm::uvec2 &dims)
    : modelViewMat(1)
    , projectionMat(1)
    , dims(dims) {
    resizeWindow(dims);
}
void HUD::add(std::shared_ptr<Overlay> overlay, AnchorV anchorV, AnchorH anchorH, const glm::ivec2 &offset, int zIndex) {
    remove(overlay);
    std::list<std::shared_ptr<Item>>::iterator it = stack.begin();
    for (; it != stack.end(); ++it) {
        // Find first element with a lower z-index
        if ((*it)->zIndex < zIndex) {
            break;
        }
    }

    std::list<std::shared_ptr<Item>>::iterator item = stack.insert(it, std::make_shared<Item>(overlay, offset, this->dims, anchorV, anchorH, zIndex));
    overlay->setHUDItem(*item);
}
unsigned int HUD::remove(std::shared_ptr<Overlay> overlay) {
    unsigned int removed = 0;
    std::list<std::shared_ptr<Item>>::iterator it = stack.begin();
    while (it != stack.end()) {
        if ((*it)->overlay == overlay) {
            stack.erase(it++);
            removed++;
        } else {
            ++it;
        }
    }
    return removed;
}
void HUD::clear() {
    stack.clear();
}
unsigned int HUD::getCount() {
    return static_cast<unsigned int>(stack.size());
}
void HUD::reload() {
    for (std::list<std::shared_ptr<Item>>::iterator it = stack.begin(); it != stack.end(); ++it)
        (*it)->overlay->_reload();
}
void HUD::render() {
    GL_CALL(glDisable(GL_DEPTH_TEST));
    GL_CALL(glEnable(GL_BLEND));
    // Iterate stack from lowest z-index to highest
    std::list<std::shared_ptr<Item>>::reverse_iterator it = stack.rbegin();
    while (it != stack.rend()) {
        (*it)->overlay->render(&modelViewMat, &projectionMat, (*it)->fvbo);
        ++it;
    }
    GL_CALL(glDisable(GL_BLEND));
    GL_CALL(glEnable(GL_DEPTH_TEST));
}
void HUD::resizeWindow(const glm::uvec2 &_dims) {
    this->dims = _dims;
    // Camera at origin looking down y axis, with up vector looking up z axis
    // Top left is origin, bottom right is (width, -height)
    // Bottom left is origin
    // Top right is (width, height)
    // Rendering the z plane 0 to -1
    projectionMat =
        glm::ortho < float>(
            0.0f, static_cast<float>(this->dims.x),
            0.0f, static_cast<float>(this->dims.y),
            0.0f, 1.0f);
    for (std::list<std::shared_ptr<Item>>::iterator it = stack.begin(); it != stack.end(); ++it)
        (*it)->resizeWindow(this->dims);
}
HUD::Item::Item(std::shared_ptr<Overlay> overlay, const int &x, const int &y, const unsigned int &window_w, const unsigned int &window_h, AnchorV anchorV, AnchorH anchorH, const int &zIndex)
    : Item(overlay, { x, y }, { window_w, window_h }, anchorV, anchorH, zIndex) { }
HUD::Item::Item(std::shared_ptr<Overlay> overlay, const glm::ivec2 &offset, const glm::uvec2 &windowDims, AnchorV anchorV, AnchorH anchorH, const int &zIndex)
    : overlay(overlay)
    , offset(offset)
    , anchorV(anchorV)
    , anchorH(anchorH)
    , zIndex(zIndex)
    , vbo(0)
    , data(0) {
    // Init vbo's
    unsigned int bufferSize = 0;
    bufferSize += 4 * sizeof(glm::vec3);  // 4 points to a quad
    bufferSize += 4 * sizeof(glm::vec2);  // 4 points to a tex coord
    data = malloc(bufferSize);
    // Setup tex coords
    glm::vec2 *texCoords = static_cast<glm::vec2*>(static_cast<void*>(static_cast<glm::vec3*>(data)+4));
    texCoords[0] = glm::vec2(0.0f, 1.0f);  // TopLeft
    texCoords[1] = glm::vec2(0.0f, 0.0f);  // BottomLeft
    texCoords[2] = glm::vec2(1.0f, 1.0f);  // TopRight
    texCoords[3] = glm::vec2(1.0f, 0.0f);  // BottomRight
    // Initalise buffer
    GL_CALL(glGenBuffers(1, &vbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, bufferSize, data, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    // Setup vertices
    resizeWindow(windowDims);
    // Link Vertex Attributes TO SHADER??!?!??!?
    Shaders::VertexAttributeDetail pos(GL_FLOAT, 3, sizeof(float));
    pos.vbo = vbo;
    pos.count = 4;
    pos.data = data;
    pos.offset = 0;
    pos.stride = 0;
    overlay->getShaders()->setPositionsAttributeDetail(pos);
    Shaders::VertexAttributeDetail texCo(GL_FLOAT, 2, sizeof(float));
    texCo.vbo = vbo;
    texCo.count = 4;
    texCo.data = texCoords;
    texCo.offset = 4*sizeof(glm::vec3);
    texCo.stride = 0;
    overlay->getShaders()->setTexCoordsAttributeDetail(texCo);
    // Setup faces
    GL_CALL(glGenBuffers(1, &fvbo));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fvbo));
    const int faces[] = { 0, 1, 2, 3 };
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4*sizeof(int), &faces, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
void HUD::Item::resizeWindow(const glm::uvec2 &dims) {
    // Track parameters, so when called from overlay we can reuse previous
    static glm::uvec2 _dims;
    if (dims.x > 0 && dims.y > 0) {
        _dims = dims;
    }
    const float depth = -0.5f;
    glm::vec3 *topLeft      = static_cast<glm::vec3*>(data);
    glm::vec3 *bottomLeft   = topLeft + 1;
    glm::vec3 *topRight     = bottomLeft + 1;
    glm::vec3 *bottomRight  = topRight + 1;
    *bottomLeft = glm::vec3(0, 0, depth);
    // Anchor horizontal
    if (anchorH == AnchorH::West)
        bottomLeft->x = 0;
    else if (anchorH == AnchorH::Center)
        bottomLeft->x = static_cast<float>(static_cast<int>((_dims.x / 2.0f) - (overlay->getWidth() / 2.0f)));  // Cast back to int to prevent tearing
    else if (anchorH == AnchorH::East)
        bottomLeft->x = static_cast<float>(_dims.x) - static_cast<float>(overlay->getWidth());
    // Anchor vertical
    if (anchorV == AnchorV::South)
        bottomLeft->y = 0;
    else if (anchorV == AnchorV::Center)
        bottomLeft->y = static_cast<float>(static_cast<int>((_dims.y / 2.0f) - (overlay->getHeight() / 2.0f)));  // Cast back to int to prevent tearing
    else if (anchorV == AnchorV::North)
        bottomLeft->y = (static_cast<float>(_dims.y) - static_cast<float>(overlay->getHeight()));
    // Apply offsets
    bottomLeft->x += offset.x;
    bottomLeft->y += offset.y;
    // Adjust other corners relative to topLeft & overlay size
    *topLeft     = glm::vec3(*bottomLeft);
    *bottomRight = glm::vec3(*bottomLeft);
    *topRight    = glm::vec3(*bottomLeft);
    topLeft->y     += overlay->getHeight();
    bottomRight->x += overlay->getWidth();
    topRight->x    += overlay->getWidth();
    topRight->y    += overlay->getHeight();
    // update data within vbo
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(glm::vec3), data));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    // If required, pass to shader
    auto pair = Shaders::findUniform("_viewportDims", overlay->getShaders()->getProgram());
    if (std::get<0>(pair) != -1) {
        glm::ivec2 viewportDims(_dims);
        overlay->getShaders()->addStaticUniform("_viewportDims", glm::value_ptr(viewportDims), 2);
    }
}
