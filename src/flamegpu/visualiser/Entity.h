#ifndef SRC_FLAMEGPU_VISUALISER_ENTITY_H_
#define SRC_FLAMEGPU_VISUALISER_ENTITY_H_
#include <memory>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "flamegpu/visualiser/util/GLcheck.h"
#include "flamegpu/visualiser/shader/Shaders.h"
#include "flamegpu/visualiser/texture/Texture2D.h"
#include "flamegpu/visualiser/interface/Renderable.h"
#include "flamegpu/visualiser/model/Material.h"
#include "flamegpu/visualiser/shader/ShadersVec.h"

namespace flamegpu {
namespace visualiser {

/*
A renderable model loaded from a .obj file
*/
class Entity : public Renderable {
    friend class Shaders;

 public:
    explicit Entity(
        const char *modelPath,
        const glm::vec3 &scale = glm::vec3(1.0f),
        Stock::Shaders::ShaderSet const ss = Stock::Shaders::FIXED_FUNCTION,
        std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr));
    explicit Entity(
        const char *modelPath,
        const glm::vec3 &modelScale,
        std::shared_ptr<Shaders> shaders = std::shared_ptr<Shaders>(nullptr),
        std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr));
    explicit Entity(
        const char *modelPath,
        const glm::vec3 &scale = glm::vec3(1.0f),
        std::initializer_list<const Stock::Shaders::ShaderSet> ss = {},
        std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr));
    Entity(
        const char *modelPath,
        const glm::vec3 &modelScale,
        std::initializer_list<std::shared_ptr<Shaders>> shaders = {},
        std::shared_ptr<const Texture> texture = std::shared_ptr<Texture2D>(nullptr));
    explicit Entity(
        const char *modelPath,
        const glm::vec3 &modelScale,
        std::vector<std::shared_ptr<Shaders>> shaders,
        std::shared_ptr<const Texture> texture);
    explicit Entity(
        const char *modelPath,
        Stock::Materials::Material const material,
        const glm::vec3 &scale = glm::vec3(1.0f));
    virtual ~Entity();
    /**
     * Loads a second model (must have the same vertex/polygon count) and attaches it to _vertex2, _normal2 within the shader
     * This is used for keyframe animations
     */
    void loadKeyFrameModel(const std::string &modelpathB);
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
    std::unique_ptr<ShadersVec> getShaders(unsigned int shaderIndex = 0) const;
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
    // World scale of each side (in the axis x, y or z)
    const glm::vec3 SCALE;
    // Model is scaled using model-mat, so we use vec4 for safety
    glm::vec4 scaleFactor;
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
            if ((s.vertex && s.vertex[0] != '\0') || (s.fragment && s.fragment[0] != '\0') || (s.geometry && s.geometry[0] != '\0'))
                rtn.push_back(std::make_shared<Shaders>(s.vertex, s.fragment, s.geometry));
        return rtn;
    }
    // Set by importModel if the imported model was of an older version.
    bool needsExport;
    bool cullFace;
    static const char *OBJ_TYPE;
    static const char *EXPORT_TYPE;
    void importModel(const char *path);
    std::unique_ptr<Entity> keyframe_model;

 private:
    struct ExportMask {
        unsigned char FILE_TYPE_FLAG;
        unsigned char VERSION_FLAG;
        unsigned char SIZE_OF_FLOAT;
        unsigned char SIZE_OF_UINT;
        float     SCALE;
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

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_ENTITY_H_
