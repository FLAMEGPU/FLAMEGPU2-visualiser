#include "Draw.h"

#include <stdexcept>

#include "flamegpu/visualiser/util/StringUtils.h"

namespace flamegpu {
namespace visualiser {

const unsigned int Draw::DEFAULT_INITIAL_VBO_LENGTH = 1024;
const float Draw::STORAGE_MUTLIPLIER = 2.0f;
Draw::Draw(const unsigned int &bufferLength, const glm::vec4 &initialColor, const float &initialWidth)
    : tName()
    , tColor(initialColor)
    , tWidth(initialWidth)
    , tType()
    , shaders(std::make_shared<Shaders>(Stock::Shaders::COLOR_NOSHADE))
    , vertices(GL_FLOAT, 3, sizeof(float))
    , colors(GL_FLOAT, 4, sizeof(float))
    , vboLen(bufferLength == 0 ? DEFAULT_INITIAL_VBO_LENGTH : bufferLength)
    , vboOffset(0)
    , requiredLength(0) {
    visassert(STORAGE_MUTLIPLIER > 1.0f);
    // Vertices vbo
    unsigned int vboSize = DEFAULT_INITIAL_VBO_LENGTH * sizeof(glm::vec3);
    GL_CALL(glGenBuffers(1, &vertices.vbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vertices.vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, vboSize, nullptr, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    // Colors vbo
    unsigned int cvboSize = DEFAULT_INITIAL_VBO_LENGTH * sizeof(glm::vec4);
    GL_CALL(glGenBuffers(1, &colors.vbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colors.vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, cvboSize, nullptr, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));

    vertices.count = DEFAULT_INITIAL_VBO_LENGTH;
    vertices.data = nullptr;

    colors.count = DEFAULT_INITIAL_VBO_LENGTH;
    colors.data = nullptr;

    shaders->setPositionsAttributeDetail(vertices, false);
    shaders->setColorsAttributeDetail(colors);
}
Draw::~Draw() {
    GL_CALL(glDeleteBuffers(1, &vertices.vbo));
    GL_CALL(glDeleteBuffers(1, &colors.vbo));
}
void Draw::begin(Type type, const std::string &name) {
    if (isDrawing) {
        THROW VisAssert("Draw::begin() cannot be called whilst the draw state is already open.\n");
    }
    isDrawing = true;
    tType = type;
    tName = name;
}
void Draw::end(bool replaceExisting) {
    if (!isDrawing) {
        THROW VisAssert("Draw::end() cannot be called whilst the draw state is not open.\n");
    }
    if (tName.empty()) {
        State s = _save(true);
        render(s);
    } else {
        std::string stateName = tName;  // Save will purge name
        save(replaceExisting);  // Forward to named draw state end to ensure it is saved
        render(stateName);
    }
}
void Draw::save(bool replaceExisting) {
    if (!isDrawing) {
        THROW VisAssert("Draw::save() cannot be called whilst the draw state is not open.\n");
    }
    if (tName.empty()) {
        THROW VisAssert("Draw::save() cannot be called to close anonymous draw states.\n");
    }
    // Save draw state
    State tState = _save(false);
    // Store draw state in map
    auto f = stateDirectory.find(tName);
    if (f != stateDirectory.end()) {
        if (!replaceExisting) {
            THROW VisAssert("Draw::save(): Overwriting draw states must be an explicit action, use Draw::save(true).\n");
        } else {
            vboGaps.push_back({ f->second.offset, f->second.count });
            requiredLength -= f->second.count;
            stateDirectory.erase(tName);
        }
    }
    requiredLength += tState.count;
    stateDirectory.insert({ tName, std::move(tState) });
}
bool Draw::has(const std::string &name) {
    return stateDirectory.find(name) != stateDirectory.end();
}
Draw::State Draw::_save(bool isTemporary) {
    if (tType == Type::Lines && tVertices.size() % 2 != 0) {
        THROW VisAssert("Draw::_save(): Line drawings require an even number of vertices.\n");
    }
    visassert(tVertices.size() == tColors.size());
    // Select the smallest gap which is big enough
    unsigned int best = UINT_MAX;
    unsigned int bestCt = UINT_MAX;
    for (unsigned int i = 0; i < vboGaps.size(); ++i) {
        unsigned int count = std::get<1>(vboGaps[i]);
        if (count > tVertices.size()) {
            if (bestCt < count)
                continue;
            best = i;
            bestCt = count;
        }
    }
    // If we found a suitable gap
    unsigned int bufferPos;
    if (best < vboGaps.size()) {  // We shall use a gap, so remove it from gap list
        unsigned int offset = std::get<0>(vboGaps[best]);
        bufferPos = offset;
        if (!isTemporary) {
            unsigned int count = std::get<1>(vboGaps[best]);
            vboGaps.erase(vboGaps.begin() + best);
            if (count >tVertices.size()) {  // Add remainder of gap back to gap list
                vboGaps.push_back({ offset + static_cast<unsigned int>(tVertices.size()), count - static_cast<unsigned int>(tVertices.size()) });
            }
        }
    } else {  // No suitable gap, resize buffer
        if (vboOffset + tVertices.size() > vboLen) {
            // Resize buffer
            unsigned int newLen = vboLen;
            while (requiredLength + tVertices.size() > newLen) {
                newLen = static_cast<unsigned int>(newLen * STORAGE_MUTLIPLIER);
            }
            resize(newLen);
        }
        bufferPos = vboOffset;
        vboOffset += static_cast<unsigned int>(tVertices.size());
    }
    // Close and package draw state
    State rtn;
    rtn.count = static_cast<unsigned int>(tVertices.size());
    rtn.offset = bufferPos;
    rtn.mType = tType;
    rtn.mWidth = tWidth;
    // Fill VBO
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vertices.vbo));
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, bufferPos*sizeof(glm::vec3), rtn.count*sizeof(glm::vec3), tVertices.data()));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colors.vbo));
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, bufferPos*sizeof(glm::vec4), rtn.count*sizeof(glm::vec4), tColors.data()));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    // Clear temporary structures
    tVertices.clear();
    tColors.clear();
    isDrawing = false;
    // Return
    return rtn;
}
void Draw::render(const std::string &name) {
    auto f = stateDirectory.find(name);
    if (f == stateDirectory.end()) {
        THROW VisAssert("Draw::render(): Draw state '%s' was not found for rendering().\n", name.c_str());
    }
    render(f->second);
}
void Draw::render(const State &state) const {
    setWidth(state.mType, state.mWidth);
    shaders->useProgram();
    GL_CALL(glDrawArrays(toGL(state.mType), state.offset, state.count));
    shaders->clearProgram();
    clearWidth(state.mType);
}
GLenum Draw::toGL(const Type &t) {
    if (t == Type::Lines) {
        return GL_LINES;
    } else if (t == Type::Polyline) {
        return GL_LINE_STRIP;
    } else if (t == Type::Points) {
        return GL_POINTS;
    }
    THROW VisAssert("Draw::toGL(): Unexpected Type pased to Draw::toGL()\n");
}
void Draw::setWidth(const Type &t, const float &w) {
    if (t == Type::Lines) {
        GL_CALL(glLineWidth(w));
    } else if (t == Type::Polyline) {
        GL_CALL(glLineWidth(w));
    } else if (t == Type::Points) {
        GL_CALL(glPointSize(w));
    } else {
        THROW VisAssert("Draw::setWidth(): Unexpected Type passed to Draw::setWidth()\n");
    }
}
void Draw::clearWidth(const Type &t) {
    if (t == Type::Lines) {
        GL_CALL(glLineWidth(1.0f));
    } else if (t == Type::Polyline) {
        GL_CALL(glLineWidth(1.0f));
    } else if (t == Type::Points) {
        GL_CALL(glPointSize(1.0f));
    } else {
        THROW VisAssert("Draw::clearWidth(): Unexpected Type passed to Draw::clearWidth()\n");
    }
}
void Draw::resize(unsigned int newLength) {
    if (newLength <= requiredLength) {
        THROW VisAssert("Draw::resize(): New length (%u) must exceed the required length (%u)\n", newLength, requiredLength);
    }
    /**
    * Allocate new vbos of double size
    */
    // Vertices vbo
    GLuint _vbo = 0, _cvbo = 0;
    unsigned int vboSize = newLength * sizeof(glm::vec3);
    GL_CALL(glGenBuffers(1, &_vbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, vboSize, nullptr, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    // Colors vbo
    unsigned int cvboSize = newLength * sizeof(glm::vec4);
    GL_CALL(glGenBuffers(1, &_cvbo));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _cvbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, cvboSize, nullptr, GL_STATIC_DRAW));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    /**
    * Copy back data
    */
    unsigned int _vboSize = vboLen * sizeof(glm::vec3);
    unsigned int _cvboSize = vboLen * sizeof(glm::vec4);
    glm::vec3 *_vertices = reinterpret_cast<glm::vec3 *>(malloc(_vboSize));
    glm::vec4 *_colors = reinterpret_cast<glm::vec4 *>(malloc(_cvboSize));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vertices.vbo));
    GL_CALL(glGetBufferSubData(GL_ARRAY_BUFFER, 0, _vboSize, _vertices));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colors.vbo));
    GL_CALL(glGetBufferSubData(GL_ARRAY_BUFFER, 0, _cvboSize, _colors));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    unsigned int _offset = 0;
    /**
    * Defragment from old into new
    */
    for (auto &a : stateDirectory) {
        // Copy back to vbo
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _vbo));
        GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, _offset*sizeof(glm::vec3), a.second.count*sizeof(glm::vec3), &_vertices[a.second.offset]));
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, _cvbo));
        GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, _offset*sizeof(glm::vec4), a.second.count*sizeof(glm::vec4), &_colors[a.second.offset]));
        a.second.offset = _offset;
        _offset += a.second.count;
    }
    if (_offset != requiredLength) {
        THROW VisAssert("Draw::resize(): _offset (%u) != requiredLength (%u)\n", _offset, requiredLength);
    }
    vboOffset = _offset;
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    free(_vertices);
    free(_colors);
    vboGaps.clear();
    /**
    * Delete and replace old vbos
    */
    GL_CALL(glDeleteBuffers(1, &vertices.vbo));
    GL_CALL(glDeleteBuffers(1, &colors.vbo));
    vertices.vbo = _vbo;
    colors.vbo = _cvbo;
    shaders->setPositionsAttributeDetail(vertices, false);
    shaders->setColorsAttributeDetail(colors);
    vboLen = newLength;
}
void Draw::reload() {
    shaders->reload();
}
void Draw::setViewMatPtr(glm::mat4 const *viewMat) {
    shaders->setViewMatPtr(viewMat);
}
void Draw::setProjectionMatPtr(glm::mat4 const *projectionMat) {
    shaders->setProjectionMatPtr(projectionMat);
}
void Draw::setLightsBuffer(const GLuint &bufferBindingPoint) {
    shaders->setLightsBuffer(bufferBindingPoint);
}

}  // namespace visualiser
}  // namespace flamegpu
