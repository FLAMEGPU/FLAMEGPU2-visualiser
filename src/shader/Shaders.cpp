#include "shader/Shaders.h"

#include <cstdio>
#include <cstdlib>
#include <utility>
#include <stdexcept>

#pragma warning(push, 3)
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#pragma warning(pop)

const char *Shaders::MODELVIEW_MATRIX_UNIFORM_NAME = "_modelViewMat";
const char *Shaders::PROJECTION_MATRIX_UNIFORM_NAME = "_projectionMat";
const char *Shaders::MODELVIEWPROJECTION_MATRIX_UNIFORM_NAME = "_modelViewProjectionMat";
const char *Shaders::NORMAL_MATRIX_UNIFORM_NAME = "_normalMat";
const char *Shaders::MODEL_MATRIX_UNIFORM_NAME = "_modelMat";
const char *Shaders::VIEW_MATRIX_UNIFORM_NAME = "_viewMat";
const char *Shaders::LIGHT_UNIFORM_BLOCK_NAME = "_lights";
const char *Shaders::MATERIAL_UNIFORM_BLOCK_NAME = "_materials";
const char *Shaders::MATERIAL_ID_UNIFORM_NAME = "_materialID";
const char *Shaders::VERTEX_ATTRIBUTE_NAME = "_vertex";
const char *Shaders::NORMAL_ATTRIBUTE_NAME = "_normal";
const char *Shaders::COLOR_ATTRIBUTE_NAME = "_color";
const char *Shaders::TEXCOORD_ATTRIBUTE_NAME = "_texCoords";
// const char *Shaders::PREV_MODELVIEW_MATRIX_UNIFORM_NAME = "_prevModelViewMat";

Shaders::Shaders(Stock::Shaders::ShaderSet set)
    :Shaders(set.vertex, set.fragment, set.geometry) {}
Shaders::Shaders(const char *vertexShaderPath, const char *fragmentShaderPath, const char *geometryShaderPath)
    : Shaders(
        vertexShaderPath ? std::initializer_list<const char *>{vertexShaderPath } : std::initializer_list<const char *>{},
        fragmentShaderPath ? std::initializer_list<const char *>{fragmentShaderPath } : std::initializer_list<const char *>{},
        geometryShaderPath ? std::initializer_list<const char *>{geometryShaderPath } : std::initializer_list<const char *>{}
    ) { }
Shaders::Shaders(std::initializer_list<const char *> vertexShaderPath, std::initializer_list<const char *> fragmentShaderPath, std::initializer_list<const char *> geometryShaderPath)
    : ShaderCore()
    , fbo(0)
    , vao(0)
    , modelMat()
    , viewMat()
    , projectionMat()
    , materialIDLocation(-1)
    , materialIDVal(INT_MAX)
    , modelviewprojectionMatLoc(-1)
    , modelviewMatLoc(-1)
    , normalMatLoc(-1)
    , rotationPtr(nullptr)
    , translationPtr(nullptr)
    , positions(GL_FLOAT, 3, sizeof(float))
    , normals(GL_FLOAT, NORMALS_SIZE, sizeof(float))
    , colors(GL_FLOAT, 3, sizeof(float))
    , texcoords(GL_FLOAT, 2, sizeof(float))
    , colorUniformLocation(-1)
    , colorUniformValue(1, 0, 0, 1)  // Red
    , vertexShaderFiles(buildFileVector(vertexShaderPath))
    , fragmentShaderFiles(buildFileVector(fragmentShaderPath))
    , geometryShaderFiles(buildFileVector(geometryShaderPath))
    , vertexShaderVersion(-1)
    , fragmentShaderVersion(-1)
    , geometryShaderVersion(-1) {
    GL_CALL(glGenVertexArrays(1, &vao));
    reload();
}
Shaders::Shaders(const Shaders &other)
    : ShaderCore(other)
    , fbo(other.fbo)
    , vao(0)
    , modelMat(UniformMatrixDetail(-1, other.modelMat.matrixPtr))
    , viewMat(UniformMatrixDetail(-1, other.viewMat.matrixPtr))
    , projectionMat(UniformMatrixDetail(-1, other.projectionMat.matrixPtr))
    , materialIDLocation(-1)
    , materialIDVal(other.materialIDVal)
    , modelviewprojectionMatLoc(-1)
    , modelviewMatLoc(-1)
    , normalMatLoc(-1)
    , rotationPtr(other.rotationPtr)
    , translationPtr(other.translationPtr)
    , positions(other.positions)
    , normals(other.normals)
    , colors(other.colors)
    , texcoords(other.texcoords)
    , colorUniformLocation(-1)
    , colorUniformSize(other.colorUniformSize)
    , colorUniformValue(other.colorUniformValue)
    , vertexShaderFiles(nullptr)
    , fragmentShaderFiles(nullptr)
    , geometryShaderFiles(nullptr)
    , vertexShaderVersion(-1)
    , fragmentShaderVersion(-1)
    , geometryShaderVersion(-1) {
    // positions, normals, colors, texcoords
    positions.location = -1;
    normals.location = -1;
    colors.location = -1;
    texcoords.location = -1;
    // gvads, lostGvads
    for (const auto &i : other.gvads)
        this->lostGvads.push_back(GenericVAD(i));
    for (const auto &i : other.lostGvads)
        this->lostGvads.push_back(GenericVAD(i));
    for (auto &i : this->lostGvads)
        i.location = -1;
    // Rebind everything to VAO
    GL_CALL(glGenVertexArrays(1, &vao));
    buildVAO();
    // fragShaderOutputLocations
    for (const auto &i : other.fragShaderOutputLocations)
        this->fragShaderOutputLocations.insert({ i.first, std::string(i.second) });
    // file vectors
    vertexShaderFiles = new std::vector<std::string>();
    for (const auto &i : *other.vertexShaderFiles)
        vertexShaderFiles->push_back(std::string(i));
    fragmentShaderFiles = new std::vector<std::string>();
    for (const auto &i : *other.fragmentShaderFiles)
        fragmentShaderFiles->push_back(std::string(i));
    geometryShaderFiles = new std::vector<std::string>();
    for (const auto &i : *other.geometryShaderFiles)
        geometryShaderFiles->push_back(std::string(i));
    // Usual shader reload
    reload();
}
Shaders::~Shaders() {
    this->destroyProgram();
    GL_CALL(glDeleteVertexArrays(1, &vao));
    delete vertexShaderFiles;
    delete fragmentShaderFiles;
    delete geometryShaderFiles;
}
bool Shaders::hasVertexShader() const {
    return this->vertexShaderFiles->size() > 0;
}
bool Shaders::hasFragmentShader() const {
    return this->fragmentShaderFiles->size() > 0;
}
bool Shaders::hasGeometryShader() const {
    return this->geometryShaderFiles->size() > 0;
}
bool Shaders::_compileShaders(const GLuint t_programId) {
    if (vertexShaderFiles->size() > 0) {
        this->vertexShaderVersion = compileShader(t_programId, GL_VERTEX_SHADER, vertexShaderFiles);
        if (this->vertexShaderVersion < 0)
            return false;
    }
    if (fragmentShaderFiles->size() > 0) {
        this->fragmentShaderVersion = compileShader(t_programId, GL_FRAGMENT_SHADER, fragmentShaderFiles);
        if (this->fragmentShaderVersion < 0) {
            this->vertexShaderVersion = -1;
            return false;
        }
    }
    if (geometryShaderFiles->size() > 0) {
        this->geometryShaderVersion = compileShader(t_programId, GL_GEOMETRY_SHADER, geometryShaderFiles);
        if (this->geometryShaderVersion < 0) {
            this->vertexShaderVersion = -1;
            this->fragmentShaderVersion = -1;
            return false;
        }
    }
    // Bind any frag shader outputs prior to shader link
    for (auto &&it : fragShaderOutputLocations) {
        GL_CALL(glBindFragDataLocation(this->getProgram(), it.first, it.second.c_str()));
    }
    return true;
}

void Shaders::bindUniform(int *rtn, const char *uniformName, GLenum uniformType) const {
    // Locate the view matrix uniform (modelview matrix, before model-specific transformations are applied)
    std::pair<int, GLenum> u_M = findUniform(uniformName, this->getProgram());
    if (u_M.first >= 0 && (u_M.second == uniformType))
        *rtn = u_M.first;
    else
        *rtn = -1;
}
void Shaders::bindAttribute(int *rtn, const char *attributeName, GLenum attributeType1, GLenum attributeType2) const {
    std::pair<int, GLenum> a_V = findAttribute(attributeName, this->getProgram());
    if (a_V.first >= 0 && (a_V.second == attributeType1 || a_V.second == attributeType2))
        *rtn = a_V.first;
    else
        *rtn = -1;
}
void Shaders::_setupBindings() {
    // Check whether shader supports blend (has it got an output alpha channel?)
    {
        this->supportsBlend = false;
        GLint numOutputs = 0;
        GL_CALL(glGetProgramInterfaceiv(this->getProgram(), GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &numOutputs));
        const GLenum properties[2] = { GL_TYPE, GL_REFERENCED_BY_FRAGMENT_SHADER };
        for (int i = 0; i < numOutputs; ++i) {
            GLint vals[2] = { 0, 0 };
            GL_CALL(glGetProgramResourceiv(this->getProgram(), GL_PROGRAM_OUTPUT, i, 2, properties, 2, nullptr, vals));
            if (vals[1] != 0) {  // Output from frag shader
                if (vals[0] == GL_FLOAT_VEC4
                    || vals[0] == GL_DOUBLE_VEC4
                    || vals[0] == GL_INT_VEC4
                    || vals[0] == GL_UNSIGNED_INT_VEC4
                    || vals[0] == GL_BOOL_VEC4
                    ) {  // Shader has an alpha channel
                    this->supportsBlend = true;
                    break;
                }
            }
        }
    }
    // MVP
    bindUniform(&this->modelMat.location, MODEL_MATRIX_UNIFORM_NAME, GL_FLOAT_MAT4);
    bindUniform(&this->viewMat.location, VIEW_MATRIX_UNIFORM_NAME, GL_FLOAT_MAT4);
    bindUniform(&this->projectionMat.location, PROJECTION_MATRIX_UNIFORM_NAME, GL_FLOAT_MAT4);
    // MVP - Convenience
    bindUniform(&this->modelviewprojectionMatLoc, MODELVIEWPROJECTION_MATRIX_UNIFORM_NAME, GL_FLOAT_MAT4);
    bindUniform(&this->modelviewMatLoc, MODELVIEW_MATRIX_UNIFORM_NAME, GL_FLOAT_MAT4);
    bindUniform(&this->normalMatLoc, NORMAL_MATRIX_UNIFORM_NAME, GL_FLOAT_MAT3);
    // bindUniform(&this->prevModelviewUniformLocation, PREV_MODELVIEW_MATRIX_UNIFORM_NAME, GL_FLOAT_MAT4);
    // Vertex attribs
    bindAttribute(&this->positions.location, VERTEX_ATTRIBUTE_NAME, GL_FLOAT_VEC3, GL_FLOAT_VEC4);
    bindAttribute(&this->normals.location, NORMAL_ATTRIBUTE_NAME, GL_FLOAT_VEC3, GL_FLOAT_VEC4);
    bindAttribute(&this->colors.location, COLOR_ATTRIBUTE_NAME, GL_FLOAT_VEC3, GL_FLOAT_VEC4);
    bindAttribute(&this->texcoords.location, TEXCOORD_ATTRIBUTE_NAME, GL_FLOAT_VEC2, GL_FLOAT_VEC3);
    // Material ID uniform
    bindUniform(&this->materialIDLocation, MATERIAL_ID_UNIFORM_NAME, GL_UNSIGNED_INT);
    if (this->materialIDLocation != -1) overrideMaterialID(this->materialIDVal);
    // Locate the color uniform
    std::pair<int, GLenum> u_C = findUniform(COLOR_ATTRIBUTE_NAME, this->getProgram());
    if (u_C.first >= 0 && (u_C.second == GL_FLOAT_VEC3 || u_C.second == GL_FLOAT_VEC4)) {
        this->colorUniformLocation = u_C.first;
        this->colorUniformSize = u_C.second == GL_FLOAT_VEC3 ? 3 : 4;
        setColor(this->colorUniformValue);
    } else {
        this->colorUniformLocation = -1;
        this->colorUniformSize = u_C.second == GL_FLOAT_VEC3 ? 3 : 4;
    }
    // Refresh generic vertex attribs
    std::list<GenericVAD> t_gvad;
    t_gvad.splice(t_gvad.end(), lostGvads);
    t_gvad.splice(t_gvad.end(), gvads);
    for (GenericVAD gvad : t_gvad) {
        std::pair<int, GLenum> a_G = findAttribute(gvad.attributeName, this->getProgram());
        if (a_G.first >= 0) {
            gvad.location = a_G.first;
            gvads.push_back(gvad);
        } else {
            lostGvads.push_front(gvad);
            fprintf(stderr, "%s: Generic vertex attrib named: %s could not be located on shader reload.\n", this->getShaderTag(), gvad.attributeName);
        }
    }
    buildVAO();
}
void Shaders::setFaceVBO(GLuint _fbo) {
    this->fbo = _fbo;
    buildVAO();
}
void Shaders::overrideModelMat(const glm::mat4 *force) {
#ifdef _DEBUG
    int currProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    if (currProgram != getProgram() || getProgram() == -1) {
        throw std::runtime_error("Error: Shader::overrideModelMat() should only be called whilst the shader is in use.\n");
    }
#endif
    _useProgramModelMatrices(force);
}
void Shaders::overrideMaterialID(unsigned int materialIndex) {
#ifdef _DEBUG
    int currProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
    if (currProgram != getProgram() || getProgram() == -1) {
        throw std::runtime_error("Error: Shader::overrideMaterialID() should only be called whilst the shader is in use.\n");
    }
#endif
    materialIDVal = materialIndex;
    // Set the color uniform if present
    if (this->getProgram() > 0 && this->materialIDLocation >= 0) {  // If colour uniform location is known
        GL_CALL(glUniform1ui(this->materialIDLocation, materialIDVal));
    }
}
void Shaders::setMaterialID(unsigned int materialIndex) {
    materialIDVal = materialIndex;
    // Set the color uniform if present
    if (this->getProgram() > 0 && this->materialIDLocation >= 0) {  // If colour uniform location is known
        GL_CALL(glUseProgram(this->getProgram()));
        GL_CALL(glUniform1ui(this->materialIDLocation, materialIDVal));
        GL_CALL(glUseProgram(0));
    }
}
void Shaders::_useProgramModelMatrices(const glm::mat4 *force) {
    // // Set the previous modelview matrix (e.g. glFrustum, normally provided by the Visualisation)
    // if (prevModelviewUniformLocation >= 0)
    // {  // If previous modelview matrix location is known
    //     GL_CALL(glUniformMatrix4fv(prevModelviewUniformLocation, 1, GL_FALSE, glm::value_ptr(prevModelview)));
    // }
    // Calculate model matrix transformations
    glm::mat4 m(1);
    // If this has been triggered as an override, ignore this->modelMat.matrixPtr
    if (force) {
        m = *force;
    }  else {  // Else calculate the required modelMat
        if (this->modelMat.matrixPtr) {
            m = *this->modelMat.matrixPtr;
        }
        if (this->rotationPtr) {
            // Check we actually have a rotation (providing no axis == error)
            if ((this->rotationPtr->x != 0 || this->rotationPtr->y != 0 || this->rotationPtr->z != 0) && this->rotationPtr->w != 0)
                m = glm::rotate(m, glm::radians(this->rotationPtr->w), glm::vec3(*this->rotationPtr));
        }
        if (this->translationPtr) {
            m = glm::translate(m, *this->translationPtr);
        }
    }

    // Set Model matrix
    if (this->modelMat.location >= 0) {  // If model matrix location is known
        GL_CALL(glUniformMatrix4fv(this->modelMat.location, 1, GL_FALSE, glm::value_ptr(m)));
    }

    // Convert model matrix into modelview
    if (this->viewMat.matrixPtr)
        m = *this->viewMat.matrixPtr * m;
    // Set the model view matrix
    if (this->modelviewMatLoc >= 0) {  // If modeview matrix location and camera ptr are known
        GL_CALL(glUniformMatrix4fv(this->modelviewMatLoc, 1, GL_FALSE, glm::value_ptr(m)));
    }

    // Sets the normal matrix (this must occur after modelView transformations are calculated)
    if (normalMatLoc >= 0) {  // If normal matrix location and modelview ptr are known
        glm::mat3 nm = glm::inverseTranspose(glm::mat3(m));
        GL_CALL(glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, glm::value_ptr(nm)));
    }

    // Set the model view projection matrix (e.g. projection * modelview)
    if (this->modelviewprojectionMatLoc >= 0 && this->projectionMat.matrixPtr) {
        m = *this->projectionMat.matrixPtr * m;
        GL_CALL(glUniformMatrix4fv(this->modelviewprojectionMatLoc, 1, GL_FALSE, glm::value_ptr(m)));
    }
}
void Shaders::buildVAO() {
    GL_CALL(glBindVertexArray(vao));
    GLuint activeVBO = 0;
    // Set the vertex (location) attribute
    if (this->positions.location >= 0 && this->positions.vbo > 0) {  // If vertex attribute location and vbo are known
        GL_CALL(glEnableVertexAttribArray(this->positions.location));
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->positions.vbo));
        if (this->positions.componentType == GL_FLOAT || this->positions.componentType == GL_HALF_FLOAT) {
            GL_CALL(glVertexAttribPointer(this->positions.location, this->positions.components, this->positions.componentType, GL_FALSE, this->positions.stride, static_cast<char *>(nullptr) + this->positions.offset));
        } else if (this->positions.componentType == GL_DOUBLE) {
            GL_CALL(glVertexAttribLPointer(this->positions.location, this->positions.components, this->positions.componentType, this->positions.stride, static_cast<char *>(nullptr) + this->positions.offset));
        } else {
            GL_CALL(glVertexAttribIPointer(this->positions.location, this->positions.components, this->positions.componentType, this->positions.stride, static_cast<char *>(nullptr) + this->positions.offset));
        }
        activeVBO = this->positions.vbo;
    }

    // Set the vertex normal attribute
    if (this->normals.location >= 0 && this->normals.vbo > 0) {  // If vertex attribute location and vbo are known
        GL_CALL(glEnableVertexAttribArray(this->normals.location));
        if (activeVBO != this->normals.vbo) {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->normals.vbo));
            activeVBO = this->normals.vbo;
        }
        if (this->normals.componentType == GL_FLOAT || this->normals.componentType == GL_HALF_FLOAT) {
            GL_CALL(glVertexAttribPointer(this->normals.location, this->normals.components, this->positions.componentType, GL_FALSE, this->normals.stride, static_cast<char *>(nullptr) + this->normals.offset));
        } else if (this->normals.componentType == GL_DOUBLE) {
            GL_CALL(glVertexAttribLPointer(this->normals.location, this->normals.components, this->positions.componentType, this->normals.stride, static_cast<char *>(nullptr) + this->normals.offset));
        } else {
            GL_CALL(glVertexAttribIPointer(this->normals.location, this->normals.components, this->positions.componentType, this->normals.stride, static_cast<char *>(nullptr) + this->normals.offset));
        }
    }
    // Set the vertex color attributes
    if (this->colors.location >= 0 && this->colors.vbo > 0) {  // If color attribute location and vbo are known
        GL_CALL(glEnableVertexAttribArray(this->colors.location));
        if (activeVBO != this->colors.vbo) {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->colors.vbo));
            activeVBO = this->colors.vbo;
        }
        if (this->colors.componentType == GL_FLOAT || this->colors.componentType == GL_HALF_FLOAT) {
            GL_CALL(glVertexAttribPointer(this->colors.location, this->colors.components, this->colors.componentType, GL_FALSE, this->colors.stride, static_cast<char *>(nullptr) + this->colors.offset));
        } else if (this->colors.componentType == GL_DOUBLE) {
            GL_CALL(glVertexAttribLPointer(this->colors.location, this->colors.components, this->colors.componentType, this->colors.stride, static_cast<char *>(nullptr) + this->colors.offset));
        } else {
            GL_CALL(glVertexAttribIPointer(this->colors.location, this->colors.components, this->colors.componentType, this->colors.stride, static_cast<char *>(nullptr) + this->colors.offset));
        }
    }

    // Set the vertex texture coord attributes
    if (this->texcoords.location >= 0 && this->texcoords.vbo > 0) {  // If texture attribute location and vbo are known
        GL_CALL(glEnableVertexAttribArray(this->texcoords.location));
        if (activeVBO != this->texcoords.vbo) {
            GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, this->texcoords.vbo));
            activeVBO = this->texcoords.vbo;
        }
        if (this->texcoords.componentType == GL_FLOAT || this->texcoords.componentType == GL_HALF_FLOAT) {
            GL_CALL(glVertexAttribPointer(this->texcoords.location, this->texcoords.components, this->texcoords.componentType, GL_FALSE, this->texcoords.stride, static_cast<char *>(nullptr) + this->texcoords.offset));
        } else if (this->texcoords.componentType == GL_DOUBLE) {
            GL_CALL(glVertexAttribLPointer(this->texcoords.location, this->texcoords.components, this->texcoords.componentType, this->texcoords.stride, static_cast<char *>(nullptr) + this->texcoords.offset));
        } else {
            GL_CALL(glVertexAttribIPointer(this->texcoords.location, this->texcoords.components, this->texcoords.componentType, this->texcoords.stride, static_cast<char *>(nullptr) + this->texcoords.offset));
        }
    }

    // Generics
    for (GenericVAD const &a : gvads) {
        // Set the generic vertex attributes
        if (a.location >= 0 && a.vbo > 0) {  // If generic vertex attribute location and vbo are known
            GL_CALL(glEnableVertexAttribArray(a.location));
            if (activeVBO != a.vbo) {
                GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, a.vbo));
                activeVBO = a.vbo;
            }
            if (a.componentType == GL_FLOAT || a.componentType == GL_HALF_FLOAT) {
                GL_CALL(glVertexAttribPointer(a.location, a.components, a.componentType, GL_FALSE, a.stride, static_cast<char *>(nullptr) + a.offset));
            } else if (a.componentType == GL_DOUBLE) {
                GL_CALL(glVertexAttribLPointer(a.location, a.components, a.componentType, a.stride, static_cast<char *>(nullptr) + a.offset));
            } else {
                GL_CALL(glVertexAttribIPointer(a.location, a.components, a.componentType, a.stride, static_cast<char *>(nullptr) + a.offset));
            }
        }
    }
    // Face vbo
    if (fbo)
        GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbo));
    GL_CALL(glBindVertexArray(0));
}
// Overrides
void Shaders::_prepare() {
    _useProgramModelMatrices();
    // Set the projection matrix (e.g. glFrustum, normally provided by the Visualisation)
    if (this->vertexShaderVersion <= 120 && this->projectionMat.matrixPtr > nullptr) {  // If old shaders where gl_ModelViewProjectionMatrix is available
        GL_CALL(glMatrixMode(GL_PROJECTION));
        GL_CALL(glLoadMatrixf(glm::value_ptr(*this->projectionMat.matrixPtr)));
    }
    if (this->projectionMat.location >= 0 && this->projectionMat.matrixPtr > nullptr) {  // If projection matrix location and camera ptr are known
        GL_CALL(glUniformMatrix4fv(this->projectionMat.location, 1, GL_FALSE, glm::value_ptr(*this->projectionMat.matrixPtr)));
    }
    // Set the view matrix (e.g. gluLookAt, normally provided by the Camera)
    if (this->viewMat.location >= 0 && this->viewMat.matrixPtr > nullptr) {  // If view matrix location and camera ptr are known
        GL_CALL(glUniformMatrix4fv(this->viewMat.location, 1, GL_FALSE, glm::value_ptr(*this->viewMat.matrixPtr)));
    }
}
void Shaders::_useProgram() {
    GL_CALL(glBindVertexArray(vao));
}
void Shaders::_clearProgram() {
    GL_CALL(glBindVertexArray(0));
}
// Bindings
void Shaders::setPositionsAttributeDetail(VertexAttributeDetail vad, bool update) {
    vad.location = this->positions.location;
    this->positions = vad;
    assert(this->positions.vbo > 0);  // vbo must be set, else we wont render anything!
    if (update)
        buildVAO();
}
void Shaders::setNormalsAttributeDetail(VertexAttributeDetail vad, bool update) {
    vad.location = this->normals.location;
    this->normals = vad;
    if (update)
        buildVAO();
}
void Shaders::setColorsAttributeDetail(VertexAttributeDetail vad, bool update) {
    vad.location = this->colors.location;
    this->colors = vad;
    if (update)
        buildVAO();
}
void Shaders::setTexCoordsAttributeDetail(VertexAttributeDetail vad, bool update) {
    vad.location = this->texcoords.location;
    this->texcoords = vad;
    if (update)
        buildVAO();
}
bool Shaders::addGenericAttributeDetail(const char* attributeName, VertexAttributeDetail vad, bool update) {
    GenericVAD gvad(vad, attributeName);
    gvad.location = -1;
    // Purge any existing gvad which matches
    removeGenericAttributeDetail(attributeName);
    if (this->getProgram() > 0) {
        // Locate the vertexNormal attribute
        std::pair<int, GLenum> a_N = findAttribute(attributeName, this->getProgram());
        if (a_N.first >= 0) {
            gvad.location = a_N.first;
            gvads.push_back(gvad);
            if (update)
                buildVAO();
            buildVAO();
            return true;
        }
        fprintf(stderr, "%s: Generic vertex attrib named: %s was not found.\n", this->getShaderTag(), gvad.attributeName);
    }
    lostGvads.push_back(gvad);
    return false;
}
bool Shaders::removeGenericAttributeDetail(const char* attributeName) {
    bool rtn = false;
    for (auto a = lostGvads.begin(); a != lostGvads.end();) {
        if (std::string((*a).attributeName) == std::string(attributeName)) {
            a = lostGvads.erase(a);
            rtn = true;
        } else {
            ++a;
        }
    }
    for (auto a = gvads.begin(); a != gvads.end();) {
        if (std::string((*a).attributeName) == std::string(attributeName)) {
            a = gvads.erase(a);
            rtn = true;
        } else {
            ++a;
        }
    }
    return rtn;
}
void Shaders::setColor(glm::vec3 color) {
    setColor(glm::vec4(color, 1.0f));
}
void Shaders::setColor(glm::vec4 color) {
    colorUniformValue = color;
    // Set the color uniform if present
    if (this->getProgram() > 0 && this->colorUniformLocation >= 0) {  // If colour uniform location is known
        GL_CALL(glUseProgram(this->getProgram()));
        if (this->colorUniformSize == 3) {
            GL_CALL(glUniform3fv(this->colorUniformLocation, 1, glm::value_ptr(color)));
        } else {
            GL_CALL(glUniform4fv(this->colorUniformLocation, 1, glm::value_ptr(color)));
        }
        GL_CALL(glUseProgram(0));
    }
}
bool Shaders::setFragOutAttribute(GLuint attachmentPoint, const char *name) {
    // Bind
    glBindFragDataLocation(this->getProgram(), attachmentPoint, name);
    // Manually check for error
    GLuint error = glGetError();
    if (error != GL_NO_ERROR) {
        if (error == GL_INVALID_VALUE)
            printf("%s(%i) GL Error Occurred;\n%s\n", __FILE__, __LINE__, reinterpret_cast<const char*>(gluErrorString(error)));
#ifdef EXIT_ON_ERROR
        getchar();
        exit(1);
#endif
        return false;
    }
    // If successful, log the attachment pt
    fragShaderOutputLocations[attachmentPoint] = std::string(name);
    // Relink the program and ensure the program re-linked correctly;
    GL_CALL(glLinkProgram(this->getProgram()));
    if (this->checkProgramLinkError(this->getProgram())) {
        setupBindings();
        return true;
    }
    return false;
}
