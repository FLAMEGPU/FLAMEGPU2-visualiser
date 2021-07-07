#ifndef SRC_FLAMEGPU_VISUALISER_SHADER_SHADERSVEC_H_
#define SRC_FLAMEGPU_VISUALISER_SHADER_SHADERSVEC_H_
#include <memory>
#include <vector>

#include "flamegpu/visualiser/shader/Shaders.h"

namespace flamegpu {
namespace visualiser {

/**
* Basic class allowing common shader operations to be perfomed across a set of shaders
*/
class ShadersVec {
    std::vector<std::shared_ptr<Shaders>> vec;

 public:
    void add(std::shared_ptr<Shaders> shaders) {
        vec.push_back(shaders);
    }
    void setViewMatPtr(const glm::mat4  *viewMat) {
        for (auto s : vec)
            s->setViewMatPtr(viewMat);
    }
    void setProjectionMatPtr(const glm::mat4 *projectionMat) {
        for (auto s : vec)
            s->setProjectionMatPtr(projectionMat);
    }
    void setModelMatPtr(const glm::mat4 *modelMat) {
        for (auto s : vec)
            s->setModelMatPtr(modelMat);
    }
    void setMaterialBuffer(GLuint materialName) {
        for (auto s : vec)
            s->setMaterialBuffer(materialName);
    }
    void setMaterialBuffer(const std::shared_ptr<UniformBuffer> &buffer) {
        for (auto s : vec)
            s->setMaterialBuffer(buffer);
    }
    void setRotationPtr(const glm::vec4 *rotationPtr) {
        for (auto s : vec)
            s->setRotationPtr(rotationPtr);
    }
    void setTranslationPtr(const glm::vec3 *translationPtr) {
        for (auto s : vec)
            s->setTranslationPtr(translationPtr);
    }
    bool addGenericAttributeDetail(const char* attributeName, Shaders::VertexAttributeDetail vad) {
        bool a = true;
        for (auto s : vec)
            a = a && s->addGenericAttributeDetail(attributeName, vad);
        return a;
    }
    void clearModelMatPtr() {
        for (auto s : vec)
            s->clearModelMatPtr();
    }
    void clearViewMatPtr() {
        for (auto s : vec)
            s->clearViewMatPtr();
    }
    void clearProjectionMatPtr() {
        for (auto s : vec)
            s->clearProjectionMatPtr();
    }
    void setColor(glm::vec3 color) {
        for (auto s : vec)
            s->setColor(color);
    }
    void setColor(glm::vec4 color) {
        for (auto s : vec)
            s->setColor(color);
    }
    bool addTexture(const char *textureNameInShader, GLenum type, GLint textureName, GLuint textureUnit) {
        bool a = true;
        for (auto s : vec)
            a = a && s->addTexture(textureNameInShader, type, textureName, textureUnit);
        return a;
    }
    bool addTexture(const char *textureNameInShader, const std::shared_ptr<const Texture> &texture) {
        bool a = true;
        for (auto s : vec)
            a = a && s->addTexture(textureNameInShader, texture);
        return a;
    }
    bool addDynamicUniform(const char *uniformName, const GLfloat *arry, unsigned int count = 1) {
        bool a = true;
        for (auto s : vec)
            a = a && s->addDynamicUniform(uniformName, arry, count);
        return a;
    }
    bool addDynamicUniform(const char *uniformName, const GLint *arry, unsigned int count = 1) {
        bool a = true;
        for (auto s : vec)
            a = a && s->addDynamicUniform(uniformName, arry, count);
        return a;
    }
    bool addDynamicUniform(const char *uniformName, const GLuint *arry, unsigned int count = 1) {
        bool a = true;
        for (auto s : vec)
            a = a && s->addDynamicUniform(uniformName, arry, count);
        return a;
    }
    bool addDynamicUniform(const char *uniformName, const glm::mat4 *mat) {
        bool a = true;
        for (auto s : vec)
            a = a && s->addDynamicUniform(uniformName, mat);
        return a;
    }
    bool addStaticUniform(const char *uniformName, const GLfloat *arry, unsigned int count = 1) {
        bool a = true;
        for (auto s : vec)
            a = a && s->addDynamicUniform(uniformName, arry, count);
        return a;
    }
    bool addStaticUniform(const char *uniformName, const GLint *arry, unsigned int count = 1) {
        bool a = true;
        for (auto s : vec)
            a = a && s->addStaticUniform(uniformName, arry, count);
        return a;
    }
    bool addStaticUniform(const char *uniformName, const GLuint *arry, unsigned int count = 1) {
        bool a = true;
        for (auto s : vec)
            a = a && s->addStaticUniform(uniformName, arry, count);
        return a;
    }
    bool addBuffer(const char *bufferNameInShader, const std::shared_ptr<BufferCore> &buffer) {
        bool a = true;
        for (auto s : vec)
            a = a && s->addBuffer(bufferNameInShader, buffer);
        return a;
    }
    bool addBuffer(const char *bufferNameInShader, const GLenum bufferType, const GLuint bufferBindingPoint) {
        bool a = true;
        for (auto s : vec)
            a = a && s->addBuffer(bufferNameInShader, bufferType, bufferBindingPoint);
        return a;
    }
    bool removeDynamicUniform(const char *uniformName) {
        bool a = true;
        for (auto s : vec)
            a = a && s->removeDynamicUniform(uniformName);
        return a;
    }
    bool removeStaticUniform(const char *uniformName) {
        bool a = true;
        for (auto s : vec)
            a = a && s->removeStaticUniform(uniformName);
        return a;
    }
    bool removeTextureUniform(const char *uniformName) {
        bool a = true;
        for (auto s : vec)
            a = a && s->removeTextureUniform(uniformName);
        return a;
    }
    bool removeBuffer(const char *nameInShader) {
        bool a = true;
        for (auto s : vec)
            a = a && s->removeBuffer(nameInShader);
        return a;
    }
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_SHADER_SHADERSVEC_H_
