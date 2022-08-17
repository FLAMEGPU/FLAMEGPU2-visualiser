#ifndef SRC_FLAMEGPU_VISUALISER_MODEL_MATERIAL_H_
#define SRC_FLAMEGPU_VISUALISER_MODEL_MATERIAL_H_

#include <utility>
#include <string>
#include <map>
#include <memory>
#include <vector>

#include "flamegpu/visualiser/texture/Texture.h"
#include "flamegpu/visualiser/interface/Renderable.h"
#include "flamegpu/visualiser/shader/Shaders.h"
#include "flamegpu/visualiser/shader/ShaderHeader.h"
#include "flamegpu/visualiser/shader/buffer/UniformBuffer.h"

namespace flamegpu {
namespace visualiser {

namespace Stock {
namespace Materials {
struct Material {
    Material(const char *name, const glm::vec3 &ambient, const glm::vec3 &diffuse, const glm::vec3 &specular, const float &shininess, const float &opacity = 1.0f)
        : name(name)
        , ambient(ambient)
        , diffuse(diffuse)
        , specular(specular)
        , shininess(shininess)
        , opacity(opacity) {}
    const char *name;
    const glm::vec3 ambient;
    const glm::vec3 diffuse;
    const glm::vec3 specular;
    const float shininess;
    const float opacity;
};
const Material WHITE{ "white", glm::vec3(0.1f), glm::vec3(1), glm::vec3(1), 76.8f, 1.0f };
const Material VANTABLACK{ "null", glm::vec3(0), glm::vec3(0), glm::vec3(0), 0.0f, 1.0f };
const Material EMERALD{ "emerald", glm::vec3(0.0215f, 0.1745f, 0.0215f), glm::vec3(0.07568f, 0.61424f, 0.07568f), glm::vec3(0.633f, 0.727811f, 0.633f), 76.8f, 1.0f };
const Material JADE{ "jade", glm::vec3(0.135f, 0.2225f, 0.1575f), glm::vec3(0.54f, 0.89f, 0.63f), glm::vec3(0.316228f), 12.8f, 1.0f };
const Material OBSIDIAN{ "obsidian", glm::vec3(0.05375f, 0.05f, 0.06625f), glm::vec3(0.18275f, 0.17f, 0.22525f), glm::vec3(0.332741f, 0.328634f, 0.346435f), 38.4f, 1.0f };
const Material PEARL{ "pearl", glm::vec3(0.25f, 0.20725f, 0.20725f), glm::vec3(1.0f, 0.829f, 0.829f), glm::vec3(0.296648f), 11.264f, 1.0f };
const Material RUBY{ "ruby", glm::vec3(0.1745f, 0.01175f, 0.01175f), glm::vec3(0.61424f, 0.04136f, 0.04136f), glm::vec3(0.727811f, 0.626959f, 0.626959f), 76.8f, 1.0f };
const Material TURQUOISE{ "turquoise", glm::vec3(0.1f, 0.18725f, 0.1745f), glm::vec3(0.396f, 0.74151f, 0.69102f), glm::vec3(0.297254f, 0.30829f, 0.306678f), 12.8f, 1.0f };
const Material BRASS{ "brass", glm::vec3(0.329412f, 0.223529f, 0.027451f), glm::vec3(0.780392f, 0.568627f, 0.113725f), glm::vec3(0.992157f, 0.941176f, 0.807843f), 27.89f, 1.0f };
const Material BRONZE{ "bronze", glm::vec3(0.2125f, 0.1275f, 0.054f), glm::vec3(0.714f, 0.4284f, 0.18144f), glm::vec3(0.393548f, 0.271906f, 0.166721f), 25.6f, 1.0f };
const Material CHROME{ "chrome", glm::vec3(0.25f), glm::vec3(0.4f), glm::vec3(0.774597f), 0.6f, 1.0f };
const Material COPPER{ "copper", glm::vec3(0.19125f, 0.0735f, 0.0225f), glm::vec3(0.7038f, 0.27048f, 0.0828f), glm::vec3(0.256777f, 0.137622f, 0.086014f), 12.8f, 1.0f };
const Material GOLD{ "gold", glm::vec3(0.24725f, 0.1995f, 0.0745f), glm::vec3(0.75164f, 0.60648f, 0.22648f), glm::vec3(0.628281f, 0.555802f, 0.366065f), 51.2f, 1.0f };
const Material SILVER{ "silver", glm::vec3(0.19225f), glm::vec3(0.50754f), glm::vec3(0.508273f), 51.2f, 1.0f };
const Material BLACK_PLASTIC{ "black plastic", glm::vec3(0), glm::vec3(0.01f), glm::vec3(0.5f), 32.0f, 1.0f };
const Material CYAN_PLASTIC{ "cyan plastic", glm::vec3(0.0f, 0.1f, 0.06f), glm::vec3(0.0f, 0.50980392f, 0.50980392f), glm::vec3(0.50196078f), 32.0f, 1.0f };
const Material GREEN_PLASTIC{ "green plastic", glm::vec3(0), glm::vec3(0.1f, 0.35f, 0.1f), glm::vec3(0.45, 0.55f, 0.45f), 32.0f, 1.0f };
const Material RED_PLASTIC{ "red plastic", glm::vec3(0), glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(0.7f, 0.6f, 0.6f), 32.0f, 1.0f };
const Material WHITE_PLASTIC{ "white plastic", glm::vec3(0), glm::vec3(0.55f), glm::vec3(0.7f), 32.0f, 1.0f };
const Material YELLOW_PLASTIC{ "yellow plastic", glm::vec3(0), glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.6f, 0.6f, 0.5f), 10.0f, 1.0f };
const Material BLACK_RUBBER{ "black rubber", glm::vec3(0.02f), glm::vec3(0.01f), glm::vec3(0.4f), 10.0f, 1.0f };
const Material CYAN_RUBBER{ "cyan rubber", glm::vec3(0.0f, 0.05f, 0.05f), glm::vec3(0.4f, 0.5f, 0.5f), glm::vec3(0.04f, 0.7f, 0.7f), 10.0f, 1.0f };
const Material GREEN_RUBBER{ "green rubber", glm::vec3(0.0f, 0.05f, 0.0f), glm::vec3(0.4f, 0.5f, 0.4f), glm::vec3(0.04f, 0.7f, 0.04f), 10.0f, 1.0f };
const Material RED_RUBBER{ "red rubber", glm::vec3(0.05f, 0.0f, 0.0f), glm::vec3(0.5f, 0.4f, 0.4f), glm::vec3(0.7f, 0.04f, 0.04f), 10.0f, 1.0f };
const Material WHITE_RUBBER{ "white rubber", glm::vec3(0.05f), glm::vec3(0.5f), glm::vec3(0.7f), 10.0f, 1.0f };
const Material YELLOW_RUBBER{ "yellow rubber", glm::vec3(0.05f, 0.05f, 0.0f), glm::vec3(0.5f, 0.5f, 0.4f), glm::vec3(0.7f, 0.7f, 0.04f), 10.0f, 1.0f };
}  // namespace Materials
}  // namespace Stock

/**
 * This class holds all the necessary data to configure a material
 * It holds a default shader specific to the material, however can be passed a shared shader with useMaterial()
 */
class Material : protected Renderable {
    /**
     * Pointer to the last used material
     * Used to prevent rebinding material states if unchanged
     * Less than ideal this being static, should be hosted in model data
     */
    static Material *active;

 public:
    enum ShadingMode {
        Constant,           // No shading, Constant light=1
        Flat,               // Shading on a per face basis, diffuse only
        Gouraud,            //
        Phong,              //
        BlinnPhong,         //
        Toon,               // Multi pass technique where edges are thick black lines and colors are often solid
        OrenNayar,          // Extends Lambertian (diffuse) reflectance to include roughness
        Minnaert,           // Extends Lambertian (diffuse) reflectance to include darkness
        CookTorrance,       // Metallic surface shader
        Fresnel             // The effect of light passing between materials
    };
    static const char * TEX_NAME[13];
    enum TextureType {
        None = 0,               // Material properties not related to textures?
        Ambient = 1,            // Combined with ambient light value
        Diffuse = 2,            // Combined with diffuse light value
        Specular = 3,           // Combined with specular light value
        Emmisive = 4,           // Added to the result of the light equation
        Height = 5,             // Height map
        Normals = 6,            // Normal map
        Shininess = 7,          // Glossiness of the material
        Opacity = 8,            // Opacity
        Displacement = 9,       // Displacement map (none-standard handling required)
        LightMap = 10,          // Ambient occlusion, how bright light is at the specified point
        Reflection = 11,        // Perfect mirror reflection (slow to calculate)
        Unknown = 12            // Unknown, will be treated as diffuse if diffuse isn't present
    };
    /**
     * Holds a Texture and it's relevant blending operations
     */
    struct TextureFrame {
        enum BlendOperation {
            Multiply,       // T1 * T2
            Add,            // T1 + T2
            Subtract,       // T1 - T2
            Divide,         // T1 / T2
            SmoothAdd,      // (T1 + T2) - (T1 * T2)
            SignedAdd,      // T1 + (T2-0.5)
            Null
        };
        std::shared_ptr<const Texture> texture = nullptr;
        float weight = 1.0f;  // This value is multiplied by the textures color
        unsigned int uvIndex = 0;  // Some meshes have dual texture coordinates
        BlendOperation operation = Add;  // How to process the n-1 texture?
        GLenum mappingModeU = GL_REPEAT;  // GL_TEXTURE_WRAP_S=GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, or GL_REPEAT.
        GLenum mappingModeV = GL_REPEAT;  // GL_TEXTURE_WRAP_T=GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, or GL_REPEAT.
        bool isDecal = false;  // Invalid tex coords cause frame dump
        bool invertColor = false;  // color=1-color
        bool useAlpha = true;  // useAlpha if present
    };
    /**
     * Creates a named material with the default configuration
     */
    Material(std::shared_ptr<UniformBuffer> &buffer, const unsigned int &bufferIndex, const char* name = "", const bool &shaderRequiresBones = false);
    /**
    * Creates a named material with the default configuration
    */
    Material(std::shared_ptr<UniformBuffer> &buffer, const unsigned int &bufferIndex, const Stock::Materials::Material &set, const bool &shaderRequiresBones = false);
    Material(const Material&other);
    /**
     *
     */
    virtual ~Material();
    /**
     * @return True if the internal configuration of the materials is identical
     * @note The material name and shaders are not considered a configuration
     */
    bool operator == (Material& other) const;
    /**
     * Prepares the indexed shader
     * @param index The index of the chosen shader, out of bound uses default for the material
     */
    void prepare(unsigned int index = UINT_MAX);
    /**
     * Enables the materials settings and selects the corresponding shader
     * @param transform The model matrix to be used
     * @param index The shader index to use, leave as default if default shader is to be used
     * @param requiresPrepare Set to true when the material's shader has not be prepared at the start of the rendercall. (This should be done if a material may be used multiple times alongside others whilst rendering a model)
     */
    void use(glm::mat4 &transform, unsigned int index = UINT_MAX, bool requiresPrepare = true);
    /**
     * Clears the indexed shader
     * @param index The index of the chosen shader, out of bound uses default for the material
     */
    void clear(unsigned int index = UINT_MAX);
    static void clearActive() { Material::active = nullptr; }
    // Setters
    void setName(const std::string _name) { this->name = _name; }

    void setDiffuse(const glm::vec3 diffuse) { this->properties.diffuse = diffuse; updatePropertiesUniform(); }
    void setSpecular(const glm::vec3 specular) { this->properties.specular = specular; updatePropertiesUniform(); }
    void setAmbient(const glm::vec3 ambient) { this->properties.ambient = ambient; updatePropertiesUniform(); }
    /*
     * Currently unused
     */
    void setEmissive(const glm::vec3 emissive) { this->properties.emissive = emissive; updatePropertiesUniform(); }
    /**
     * Currently unused
     */
    void setTransparent(const glm::vec3 transparent) { this->properties.transparent = transparent; updatePropertiesUniform(); }
    void setOpacity(const float opacity) { this->properties.opacity = opacity; updatePropertiesUniform(); }
    void setShininess(const float shininess) { this->properties.shininess = shininess; updatePropertiesUniform(); }
    /**
     * This is currently unused, unclear how it differs from shininess
     */
    void setShininessStrength(const float shininessStrength) { this->properties.shininessStrength = shininessStrength; updatePropertiesUniform(); }
    /**
     * Currently unused
     */
    void setRefractionIndex(const float refractionIndex) { this->properties.refractionIndex = refractionIndex; updatePropertiesUniform(); }

    void setWireframe(const bool _isWireframe) { this->isWireframe = _isWireframe; }
    void setTwoSided(const bool _twoSided) { this->faceCull = !_twoSided; }
    void setShadingMode(const ShadingMode _shaderMode) { this->shaderMode = _shaderMode; }
    void addTexture(TextureFrame, TextureType type = Diffuse);
    void setCustomShaders(const std::vector<std::shared_ptr<Shaders>> &shaders);
    /**
     * @see glBlendFunc()
     */
    void setAlphaBlendMode(const GLenum sfactor, const GLenum dfactor) { this->alphaBlendMode[0] = sfactor; this->alphaBlendMode[1] = dfactor; }

    // Getters
    std::string getName() const { return name; }
    glm::vec3 getDiffuse() const { return properties.diffuse; }
    glm::vec3 getSpecular() const { return properties.specular; }
    glm::vec3 getAmbient() const { return properties.ambient; }
    glm::vec3 getEmissive() const { return properties.emissive; }
    glm::vec3 getTransparent() const { return properties.transparent; }
    float getOpacity() const { return properties.opacity; }
    float getShininess() const { return properties.shininess; }
    float getShininessStrength() const { return properties.shininessStrength; }
    float getRefractionIndex() const { return properties.refractionIndex; }
    bool getWireframe() const { return isWireframe; }
    bool getTwoSided() const { return !faceCull; }
    ShadingMode getShadingMode() const { return shaderMode; }
    std::shared_ptr<Shaders> getShaders(unsigned int index = UINT_MAX) const { return index < shaders.size() ? shaders[index] : defaultShader; }
    size_t getShaderCount() const { return shaders.size(); }
    std::pair<GLenum, GLenum> getAlphaBlendMode() const { return std::make_pair(alphaBlendMode[0], alphaBlendMode[1]); }
    /**
     * Creates and sets up the default shader
     * Copies material properties to the uniform buffer for the first time
     */
    void bake();

 private:
    // Id
    std::string name;

    MaterialProperties properties;
    std::shared_ptr<UniformBuffer> buffer;
    const unsigned int bufferIndex;
    bool hasBaked;
    bool hasAlpha;
    void updatePropertiesUniform(bool force = false);
    // Textures
    std::map<TextureType, std::vector<TextureFrame>> textures;
    // Modifiers
    bool isWireframe;
    bool faceCull;
    ShadingMode shaderMode;
    std::shared_ptr<Shaders> defaultShader;
    std::vector<std::shared_ptr<Shaders>> shaders;
    GLint alphaBlendMode[2];  // GL_SRC_ALPHA, GL_ONE for additive blending// GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA for default
    bool shaderRequiresBones;
// HasMatrices overrides

 public:
    /**
     * Sets the pointer from which the View matrix should be loaded from
     * @param viewMat A pointer to the viewMatrix to be tracked
     * @note This pointer is likely provided by a Camera subclass
     */
    void setViewMatPtr(const glm::mat4  *viewMat) override;
    using Renderable::setViewMatPtr;
    /**
     * Sets the pointer from which the Projection matrix should be loaded from
     * @param projectionMat A pointer to the projectionMatrix to be tracked
     * @note This pointer is likely provided by the Visualisation object
     */
    void setProjectionMatPtr(const glm::mat4 *projectionMat) override;
    using Renderable::setProjectionMatPtr;
    /**
     * Provides lights buffer to the shader
     * @param bufferBindingPoint Set the buffer binding point to be used for rendering
     */
    void setLightsBuffer(const GLuint &bufferBindingPoint) override;
    using Renderable::setLightsBuffer;
    void reload() override;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_MODEL_MATERIAL_H_
