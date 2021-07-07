#include "flamegpu/visualiser/model/Material.h"
#include "flamegpu/visualiser/texture/Texture2D.h"

namespace flamegpu {
namespace visualiser {

Material *Material::active = nullptr;
const char * Material::TEX_NAME[13] = { "t_none", "t_ambient", "t_diffuse", "t_specular", "t_emissive", "t_height", "t_normal", "t_shininess", "t_opacity", "t_displacement", "t_light", "t_reflection", "t_unknown" };
Material::Material(std::shared_ptr<UniformBuffer> &buffer, const unsigned int &bufferIndex, const char* name, const bool &shaderRequiresBones)
    : name(name == nullptr?"":name)
    , properties()
    , buffer(buffer)
    , bufferIndex(bufferIndex)
    , hasBaked(false)
    , hasAlpha(false)
    , isWireframe(false)
    , faceCull(true)
    , shaderMode(Phong)
    , shaderRequiresBones(shaderRequiresBones) {
    // , alphaBlendMode{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }// C++11 required (maybe usable VS2015?)
    alphaBlendMode[0] = GL_SRC_ALPHA;
    alphaBlendMode[1] = GL_ONE_MINUS_SRC_ALPHA;
}

Material::Material(std::shared_ptr<UniformBuffer> &buffer, const unsigned int &bufferIndex, const Stock::Materials::Material &set, const bool &shaderRequiresBones)
    : Material(buffer, bufferIndex, set.name, shaderRequiresBones) {
    properties.ambient = set.ambient;
    properties.diffuse = set.diffuse;
    properties.specular = set.specular;
    properties.shininess = set.shininess;
    properties.opacity = set.opacity;
    bake();
}
Material::Material(const Material&other)
    : name(other.name)
    , properties(other.properties)
    , buffer(other.buffer)
    , bufferIndex(other.bufferIndex)
    , hasBaked(false)
    , hasAlpha(other.hasAlpha)
    , isWireframe(other.isWireframe)
    , faceCull(other.faceCull)
    , shaderMode(other.shaderMode)
    , shaderRequiresBones(other.shaderRequiresBones) {
    for (const auto &i : other.shaders)
        shaders.push_back(std::make_shared<Shaders>(*i));
    alphaBlendMode[0] = other.alphaBlendMode[0];
    alphaBlendMode[1] = other.alphaBlendMode[1];
    if (other.hasBaked)
        bake();
}

Material::~Material() {
}
void Material::updatePropertiesUniform(bool force) {
    // To save updating the Material buffer during it's initial construction, wait for it to be baked before allowing accessors to update
    if (!force&&!hasBaked)
        return;
    hasBaked = true;
    buffer->setData(&properties, sizeof(MaterialProperties), bufferIndex*sizeof(MaterialProperties));
}
/**
 * Todo: compare textures
 */
bool  Material::operator == (Material& other) const {
    if (
        this->properties.diffuse == other.getDiffuse() &&
        this->properties.specular == other.getSpecular() &&
        this->properties.ambient == other.getAmbient() &&
        this->properties.emissive == other.getEmissive() &&
        this->properties.transparent == other.getTransparent() &&
        this->properties.opacity == other.getOpacity() &&
        this->properties.shininess == other.getShininess() &&
        this->properties.shininessStrength == other.getShininessStrength() &&
        this->properties.refractionIndex == other.getRefractionIndex() &&
        this->isWireframe == other.getWireframe() &&
        this->faceCull == !other.getTwoSided() &&
        this->shaderMode == other.getShadingMode() &&
        this->getAlphaBlendMode() == other.getAlphaBlendMode()
        )
        return true;
    return false;
}
/**
 * Temporary texture solution before proper materials
 */
void Material::addTexture(TextureFrame texFrame, TextureType type) {
    textures[type].push_back(texFrame);
    this->properties.bitmask |= (1 << type);
    updatePropertiesUniform();
    if (hasBaked) {
        if (textures[type].size() == 1) {
            if (defaultShader)
                defaultShader->addTexture(TEX_NAME[type], texFrame.texture);
            for (const auto &i : this->shaders)
                i->addTexture(TEX_NAME[type], texFrame.texture);
        }
    }
    if (textures[type].size() > 1) {
        fprintf(stderr, "Warning: Material '%s' contains multiple textures of type %s\n Texture stacks are currently unsupported.\n", name.c_str(), TEX_NAME[type]);
    }
}
// HasMatrices overrides
void Material::setViewMatPtr(const glm::mat4 *viewMat) {
    if (defaultShader)
        defaultShader->setViewMatPtr(viewMat);
    for (const auto &i : this->shaders)
            i->setViewMatPtr(viewMat);
}
void Material::setProjectionMatPtr(const glm::mat4 *projectionMat) {
    if (defaultShader)
        defaultShader->setProjectionMatPtr(projectionMat);
    for (const auto &i : this->shaders)
        i->setProjectionMatPtr(projectionMat);
}
void Material::setLightsBuffer(const GLuint &bufferBindingPoint) {
    if (defaultShader)
        defaultShader->setLightsBuffer(bufferBindingPoint);
    for (const auto &i : this->shaders)
        i->setLightsBuffer(bufferBindingPoint);
}

void Material::setCustomShaders(const std::vector<std::shared_ptr<Shaders>> &_shaders) {
    this->shaders.clear();
    // Clone all shaders
    for (const auto &i : _shaders)
        this->shaders.push_back(std::make_shared<Shaders>(*i));
    // Setup material in all Shaders
    for (const auto &i : this->shaders) {
        // Setup material buffer
        i->setMaterialBuffer(buffer);
        i->setMaterialID(bufferIndex);
        // Setup default shader, e.g. textures
        for (auto &typeVec : textures) {
            if (typeVec.second.size())
                i->addTexture(TEX_NAME[typeVec.first], typeVec.second[0].texture);
        }
    }
}
void Material::prepare(unsigned int index) {
    if (index < shaders.size())
        shaders[index]->prepare();
    else if (defaultShader)
        defaultShader->prepare();
}
void Material::use(glm::mat4 &transform, unsigned int index, bool requiresPrepare) {
    if (index < shaders.size()) {
        shaders[index]->useProgram(requiresPrepare);
        shaders[index]->overrideModelMat(&transform);
    } else if (defaultShader) {
        defaultShader->useProgram(requiresPrepare);
        defaultShader->overrideModelMat(&transform);
    }
    if (active != this) {
        // Setup GL states for material
        if (this->isWireframe) {
            GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        } else {
            GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        if (this->faceCull) {
            GL_CALL(glEnable(GL_CULL_FACE));
        } else {
            GL_CALL(glDisable(GL_CULL_FACE));
        }
        // Treat all materials as having alpha until we can add a suitable check/switch
        // if (hasAlpha || this->properties.opacity>1.0f)
        // {
        if ((index < shaders.size() && shaders[index]->supportsGL_BLEND())
            || (index >= shaders.size() && defaultShader->supportsGL_BLEND())) {
            GL_CALL(glEnable(GL_BLEND));
            GL_CALL(glBlendFunc(alphaBlendMode[0], alphaBlendMode[1]));
        } else {
            GL_CALL(glDisable(GL_BLEND));
        }

        active = this;
#ifndef _DEBUG
    }
#else
    } else {
        // Cull face
        GLboolean cullFace;
        GL_CALL(glGetBooleanv(GL_CULL_FACE, &cullFace));  // 0 if face culling disabled
        if ((cullFace != 0) != faceCull) {
            fprintf(stderr, "Warning: Material::use() detected GL_CULL_FACE has been changed.\nUse Material::clearActive() before to prevent this warning.\n");
        }
        // Polygon mode
        GLint polygonMode[2];
        GL_CALL(glGetIntegerv(GL_POLYGON_MODE, &polygonMode[0]));
        if ((polygonMode[0] != GL_LINE&&isWireframe) || (polygonMode[0] != GL_FILL&&!isWireframe))
           fprintf(stderr, "Warning: Material::use() detected GL_POLYGON_MODE has been changed.\nUse Material::clearActive() before to prevent this warning.\n");
        // Blend mode
        GLint alphaSrc, alphaDst, rgbSrc, rgbDst;
        GL_CALL(glGetIntegerv(GL_BLEND_SRC_ALPHA, &alphaSrc));
        GL_CALL(glGetIntegerv(GL_BLEND_DST_ALPHA, &alphaDst));
        GL_CALL(glGetIntegerv(GL_BLEND_SRC_RGB, &rgbSrc));
        GL_CALL(glGetIntegerv(GL_BLEND_DST_RGB, &rgbDst));
        if (alphaSrc != alphaBlendMode[0] || rgbSrc != alphaBlendMode[0] || alphaDst != alphaBlendMode[1] || rgbDst != alphaBlendMode[1]) {
            fprintf(stderr, "Warning: Material::use() detected glBlendFunc() has been called.\nUse Material::clearActive() before to prevent this warning.\n");
        }
    }
#endif
}
void Material::clear(unsigned int index) {
    if (index < shaders.size())
        shaders[index]->clearProgram();
    else if (defaultShader)
        defaultShader->clearProgram();
}

void Material::reload() {
    if (defaultShader)
        defaultShader->reload();
    for (auto &i : shaders)
        i->reload();
}
void Material::bake() {
    // Update material properties buffer
    updatePropertiesUniform(true);
    // // Create default shader
    // if (shaderRequiresBones)
    //     defaultShader = std::make_shared<Shaders>(Stock::Shaders::BONE);  // Temp
    // else
    //     defaultShader = std::make_shared<Shaders>(Stock::Shaders::PHONG);  // Temp
    // // Setup material buffer
    // defaultShader->setMaterialBuffer(buffer);
    // defaultShader->setMaterialID(bufferIndex);
    // // Setup default shader, e.g. textures
    // for (auto &typeVec:textures)
    //     if (typeVec.second.size())
    //         defaultShader->addTexture(TEX_NAME[typeVec.first], typeVec.second[0].texture);
    // Setup material in all custom shaders
    for (const auto &i : this->shaders) {
        // Setup material buffer
        i->setMaterialBuffer(buffer);
        i->setMaterialID(bufferIndex);
        // Setup default shader, e.g. textures
        for (auto &typeVec : textures)
            if (typeVec.second.size())
                i->addTexture(TEX_NAME[typeVec.first], typeVec.second[0].texture);
    }
}

}  // namespace visualiser
}  // namespace flamegpu

