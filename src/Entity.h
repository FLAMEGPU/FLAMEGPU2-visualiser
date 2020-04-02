#ifndef SRC_ENTITY_H_
#define SRC_ENTITY_H_
#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "util/GLcheck.h"
#include "shader/Shaders.h"
#include "texture/Texture2D.h"
#include "interface/Renderable.h"
#include "model/Material.h"
#include "shader/ShadersVec.h"

namespace Stock {
namespace Models {
struct Model {
    const char *modelPath;
    const char *texturePath;
    Stock::Shaders::ShaderSet defaultShaders;
};
const Model SPHERE{ "../models/sphere.obj", "", Stock::Shaders::PHONG };
const Model ICOSPHERE{ "../models/icosphere.obj", "", Stock::Shaders::FLAT };
const Model ICOSPHERE_COLOR{ "../models/icosphere_color.obj", "", Stock::Shaders::COLOR };  // Remove texture?
const Model CUBE{ "../models/cube.obj", "", Stock::Shaders::COLOR };
const Model ROTHWELL{ "../models/rothwell-wy-1.obj", "", Stock::Shaders::PHONG };
const Model DEER{ "../models/deer.obj", "../textures/deer.tga", Stock::Shaders::PHONG };
const Model TEAPOT{ "../models/teapot.obj", "", Stock::Shaders::PHONG };
const Model PLANE{ "../models/plane.obj", "", Stock::Shaders::FLAT };
}  // namespace Models
}  // namespace Stock
/*
A renderable model loaded from a .obj file
*/
class Entity : public Renderable {
    friend class Shaders;

 public:
    explicit Entity(
        Stock::Models::Model const model,
        float scale = 1.0f,
        std::shared_ptr<Shaders> shaders = std::shared_ptr<Shaders>(nullptr),
        std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr));
    explicit Entity(
        Stock::Models::Model const model,
        float scale,
        Stock::Shaders::ShaderSet const ss,
        std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr));
    explicit Entity(
        const char *modelPath,
        float modelScale = 1.0f,
        Stock::Shaders::ShaderSet const ss = Stock::Shaders::FIXED_FUNCTION,
        std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr));
    explicit Entity(
        const char *modelPath,
        float modelScale,
        std::shared_ptr<Shaders> shaders = std::shared_ptr<Shaders>(nullptr),
        std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr));
    explicit Entity(
        Stock::Models::Model const model,
        float scale = 1.0f,
        std::initializer_list<std::shared_ptr<Shaders>> shaders = {},
        std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr));
    Entity(
        Stock::Models::Model const model,
        float scale,
        std::initializer_list<const Stock::Shaders::ShaderSet> ss,
        std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr));
    explicit Entity(
        const char *modelPath,
        float modelScale = 1.0f,
        std::initializer_list<const Stock::Shaders::ShaderSet> ss = {},
        std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr));
    explicit Entity(
        const char *modelPath,
        float modelScale,
        std::initializer_list<std::shared_ptr<Shaders>> shaders = {},
        std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr));
    explicit Entity(
        const char *modelPath,
        float modelScale,
        std::vector<std::shared_ptr<Shaders>> shaders,
        std::shared_ptr<const Texture> texture);
    explicit Entity(
        const char *modelPath,
        Stock::Materials::Material const material,
        float modelScale = 1.0f);
    explicit Entity(
        Stock::Models::Model const model,
        Stock::Materials::Material const material,
        float modelScale = 1.0f);
    virtual ~Entity();
    virtual void render(unsigned int shaderIndex = 0);
    void renderInstances(int count, unsigned int shaderIndex = 0);
    /**
     * Overrides the material in use, this will lose any textures from the exiting material
     */
    void setMaterial(const glm::vec3 &ambient, const glm::vec3 &diffuse, const glm::vec3 &specular = glm::vec3(0.1f), const float &shininess = 10.0f, const float &opacity = 1.0f);
    void setMaterial(const Stock::Materials::Material &mat);
    void setLocation(glm::vec3 location);
    void setRotation(glm::vec4 rotation);
    glm::vec3 getLocation() const;
    glm::vec4 getRotation() const;
    void exportModel() const;
    void reload() override;
    /**
     * Ensure updateShaders() is called after making changes to shaders returned by this method
     */
    std::unique_ptr<ShadersVec> Entity::getShaders(unsigned int shaderIndex = 0) const;
    void setViewMatPtr(glm::mat4 const *viewMat) override;
    using Renderable::setViewMatPtr;
    void setProjectionMatPtr(glm::mat4 const *projectionMat) override;
    using Renderable::setProjectionMatPtr;
    /**
    * Provides lights buffer to the shader
    * @param bufferBindingPoint Set the buffer binding point to be used for rendering
    */
    void setLightsBuffer(const GLuint &bufferBindingPoint) override;
    using Renderable::setLightsBuffer;
    void flipVertexOrder();
    void setCullFace(const bool cullFace);
    glm::vec3 getMin() const { return modelMin; }
    glm::vec3 getMax() const { return modelMax; }
    glm::vec3 getDimensions() const { return modelDims; }

 protected:
    glm::mat4 const * viewMatPtr;
    glm::mat4 const * projectionMatPtr;
    GLuint lightBufferBindPt;
    std::vector<std::shared_ptr<Shaders>> shaders;
    std::shared_ptr<const Texture> texture;
    // World scale of the longest side (in the axis x, y or z)
    const float SCALE;
    float scaleFactor;
    const char *modelPath;
    // Model vertex and face counts
    unsigned int vn_count;
    Shaders::VertexAttributeDetail positions, normals, colors, texcoords, faces;

    // Optional material (loaded automaically if detected within model file)
    std::vector<Material> materials;
    std::shared_ptr<UniformBuffer> materialBuffer;
    glm::vec3 location;
    glm::vec4 rotation;

    static void createVertexBufferObject(GLuint *vbo, GLenum target, GLuint size, void *data);
    static void deleteVertexBufferObject(GLuint *vbo);
    void loadModelFromFile();
    void loadMaterialFromFile(const char *objPath, const char *materialFilename, const char *materialName);
    void generateVertexBufferObjects();

 private:
    glm::mat4 getModelMat() const;
    glm::vec3 modelMin, modelMax, modelDims;
    static std::vector<std::shared_ptr<Shaders>> convertToShader(std::initializer_list<const Stock::Shaders::ShaderSet> ss) {
        std::vector<std::shared_ptr<Shaders>> rtn;
        for (auto&& s : ss)
            rtn.push_back(std::make_shared<Shaders>(s.vertex, s.fragment, s.geometry));
        return rtn;
    }
    // Set by importModel if the imported model was of an older version.
    bool needsExport;
    bool cullFace;
    static const char *OBJ_TYPE;
    static const char *EXPORT_TYPE;
    void importModel(const char *path);

 private:
    struct ExportMask {
        unsigned char FILE_TYPE_FLAG;
        unsigned char VERSION_FLAG;
        unsigned char SIZE_OF_FLOAT;
        unsigned char SIZE_OF_UINT;
        float         SCALE;
        unsigned int  VN_COUNT;
        unsigned int  FILE_HAS_VERTICES_3 : 1;
        unsigned int  FILE_HAS_VERTICES_4 : 1;
        unsigned int  FILE_HAS_NORMALS_3 : 1;  // Currently normals can only be len 3, reserved regardless
        unsigned int  FILE_HAS_COLORS_3 : 1;
        unsigned int  FILE_HAS_COLORS_4 : 1;
        unsigned int  FILE_HAS_TEXCOORDS_2 : 1;
        unsigned int  FILE_HAS_TEXCOORDS_3 : 1;
        unsigned int  FILE_HAS_FACES_3 : 1;
        unsigned int  RESERVED_SPACE : 32;
    };
    static const unsigned char FILE_TYPE_FLAG = 0x12;
    static const unsigned char FILE_TYPE_VERSION = 1;
};
#endif  // SRC_ENTITY_H_
