#ifndef SRC_SHADER_SHADERS_H_
#define SRC_SHADER_SHADERS_H_
#include <memory>
#include <list>
#include <map>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "shader/ShaderCore.h"

#define NORMALS_SIZE 3

class UniformBuffer;  // Implementation of setMaterialBuffer(const std::shared_ptr<UniformBuffer> &buffer) found in UniformBuffer.cpp

namespace Stock {
namespace Shaders {
struct ShaderSet {
    const char *vertex;
    const char *fragment;
    const char *geometry;
};
const ShaderSet FIXED_FUNCTION{ nullptr, nullptr, nullptr };
const ShaderSet FULLBRIGHT{ "resources/default.vert", "resources/fullbright_phong.frag", nullptr };
const ShaderSet FULLBRIGHT_FLAT{ "resources/default.vert", "resources/material_fullbright_flat.frag", nullptr };
const ShaderSet FULLBRIGHT_PHONG{ "resources/default.vert", "resources/material_fullbright_phong.frag", nullptr };
const ShaderSet FLAT{ "resources/default.vert", "resources/material_flat.frag", nullptr };
const ShaderSet PHONG{ "resources/default.vert", "resources/material_phong.frag", nullptr };
const ShaderSet COLOR{ "resources/color.vert", "resources/color.frag", nullptr };
const ShaderSet COLOR_NOSHADE{ "resources/color.vert", "resources/color_noshade.frag", nullptr };
const ShaderSet SKYBOX{ "resources/skybox.vert", "skybox.frag", nullptr };
const ShaderSet INSTANCED_FLAT{ "resources/instanced_flat.vert", "resources/material_flat.frag", nullptr };
const ShaderSet INSTANCED_PHONG{ "resources/instanced_default.vert", "resources/material_phong.frag", nullptr };
const ShaderSet TEXT{ "resources/default.vert", "resources/text.frag", nullptr };
const ShaderSet SPRITE2D{ "resources/default.vert", "resources/sprite2d.frag", nullptr };
const ShaderSet SPRITE2D_HEAT{ "resources/default.vert", "resources/sprite2dHeat.frag", nullptr };
const ShaderSet BILLBOARD{ "resources/billboard.vert", "resources/particle.frag", nullptr };
const ShaderSet LINEAR_DEPTH{ "resources/default.vert", "resources/linear_depth.frag", nullptr };
const ShaderSet FLAT_SHADOW{ "resources/shadow.vert", "resources/material_flat_shadow.frag", nullptr };
const ShaderSet PHONG_SHADOW{ "resources/shadow.vert", "resources/material_phong_shadow.frag", nullptr };
const ShaderSet BONE{ "resources/bone.vert", "resources/material_phong.frag", nullptr };
const ShaderSet BONE_LINEAR_DEPTH{ "resources/bone.vert", "resources/linear_depth.frag", nullptr };
const ShaderSet BONE_SHADOW{ "resources/bone_shadow.vert", "resources/material_phong_shadow.frag", nullptr };
}  // namespace Shaders
}  // namespace Stock
/**
 * Abstracts compilation of Shaders, and attempts to automatically bind provided uniforms and vertex attributes on useProgram()
 * Bindings are tied to the specific shader, if you wish to use the same shader for 2 entities it might be best to make a 2nd instance
 * @note There is an implementation specific limit of around 16 vertex attribute bindings you can have to any shader, this not enforced by this class
 * @note Floating point attribute bindings do not automatically normalise values (the parameter is always passed as false)
 * @todo Confirm that we do need re-call glVertexAttribPointer on shader re-use, or can we just bind it once
 * @todo Switch all pointers to weak_ptr
 */
class Shaders : public ShaderCore {
 public:
    // These constants are the names that will be searched for when loading shaders
    static const char *MODELVIEW_MATRIX_UNIFORM_NAME;  // ="_modelViewMat";
    static const char *PROJECTION_MATRIX_UNIFORM_NAME;  // ="_projectionMat";
    static const char *MODELVIEWPROJECTION_MATRIX_UNIFORM_NAME;  //  "_modelViewProjectionMat";
    static const char *NORMAL_MATRIX_UNIFORM_NAME;  //  = "_normalMat";
    static const char *MODEL_MATRIX_UNIFORM_NAME;  //  = "_modelMat";
    static const char *VIEW_MATRIX_UNIFORM_NAME;  //  = "_viewMat";
    static const char *LIGHT_UNIFORM_BLOCK_NAME;  //  = "_lights";
    static const char *MATERIAL_UNIFORM_BLOCK_NAME;  //  = "_materials";
    static const char *MATERIAL_ID_UNIFORM_NAME;  //  = "_materialID";
    static const char *VERTEX_ATTRIBUTE_NAME;  //  = "_vertex";
    static const char *NORMAL_ATTRIBUTE_NAME;  //  = "_normal";
    static const char *COLOR_ATTRIBUTE_NAME;  //  = "_color";
    static const char *TEXCOORD_ATTRIBUTE_NAME;  //  = "_texCoords";
    // static const char *PREV_MODELVIEW_MATRIX_UNIFORM_NAME;  //  = "_prevModelViewMat";
    /**
     * This structure represents the details necessary to correctly bind a uniform matrix (e.g. model view/projection)
     */
    struct UniformMatrixDetail {
        explicit UniformMatrixDetail(int location = -1, const glm::mat4 *matrixPtr = nullptr)
            :location(location), matrixPtr(matrixPtr) { }
        /**
         * This value is set internally when we detect that your shader has a suitable uniform location
         */
        int location;
        /**
         * Const pointer to the matrix to be loaded
         */
        const glm::mat4 *matrixPtr;
    };
    /**
     * This structure represents the details necessary to correctly bind a vertex attribute (e.g. vertex positions, normals, tex coords)
     * Most of the member vars provide a specification of how the array of vertex attributes is stored in the vbo
     * Each vertex attribute should be a vector [element of the array], and has 2-4 components
     * Multiple vertex attributes can share a vbo, with their data either packed one after the other, or interleaved.
     */
    struct VertexAttributeDetail {
        /**
         * Other values should be set after creation
         * location can be ignored as this is used internally
         */
        VertexAttributeDetail(
            GLenum componentType,
            unsigned int components,
            unsigned int componentSize
            )
            : componentType(componentType)
            , components(components)
            , componentSize(componentSize)
            , data(nullptr)
            , count(0)
            , vbo(0)
            , location(-1)
            , offset(0)
            , stride(0) {}
        /**
         * Underlying component type expressed as GLenum
         * e.g. Most cases will be float4/glm::vec4 which are GL_FLOAT
         * Options: GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE, GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, and GL_UNSIGNED_INT
         */
        GLenum componentType;
        /**
         * Number of vector components (Must be 2, 3 or 4)
         */
        unsigned int components;    // Number of vector components per attribute (Must be 2, 3 or 4)
        /**
        * Size of the component as specified in componentType
        * e.g. sizeof(float)
        */
        unsigned int componentSize;  // Both:
        /**
         * Pointer to the attribute data in memory
         */
        void *data;
        /**
         * Number of attributes in the array
         */
        unsigned int count;
        /**
         * VBO name as returned by glGenBuffers()
         */
        GLuint vbo;
        /**
         * This value is internally set when the Shader object detects a suitable binding location
         */
        int location;
        /**
         * The offset from the start of the vbo which attribute data begins
         * @note You store multiple vertex attribute arrays in the same vbo
         */
        unsigned int offset;
        /**
         * Spacing between elements within the array
         * @note This is value is 0 unless the data is interleaved
         */
        unsigned int stride;
    };
    /**
     * Constructs a shader object from one of the stock shader sets
     * @param set The shader set to create
     */
    explicit Shaders(Stock::Shaders::ShaderSet set);
    /*
     * Constructs a shader program from the provided shader files
     * @param vertexShaderFiles Path to the GLSL vertex shader (nullptr if not required)
     * @param fragmentShaderFiles Path to the GLSL fragment shader (nullptr if not required)
     * @param geometryShaderFiles Path to the GLSL geometry shader (nullptr if not required)
     * @note You must provide atleast 1 shader path, however it can be of any of the 3 types
     */
    explicit Shaders(const char *vertexShaderPath = nullptr, const char *fragmentShaderPath = nullptr, const char *geometryShaderPath = nullptr);
    /*
     * Constructs a shader program from the provided shader files
     * Use this constructor if you wish to constructor a shader from multiple files (e.g. a common functions file)
     * @param vertexShaderFiles Path to the GLSL vertex shader ({} if not required)
     * @param fragmentShaderFiles Path to the GLSL fragment shader ({} if not required)
     * @param geometryShaderFiles Path to the GLSL geometry shader ({} if not required)
     * @note You must provide atleast 1 shader path, however it can be of any of the 3 types
     */
    Shaders(std::initializer_list<std::string> vertexShaderPath, std::initializer_list<std::string> fragmentShaderPath = {}, std::initializer_list<std::string> geometryShaderPath = {});
    /**
     * Copy constructor
     * @note This does not duplicate linked external objects, e.g. buffers, textures.
     * @note There if the original shaders owning model is destroyed, this shader will point to invalid data.
     * @note Therefore this should be used with caution outside of advanced model classes
     */
    Shaders(const Shaders &other);
    /**
     * Free the shader program
     */
    ~Shaders();
    /**
     * @return True if this shader object has a vertex shader
     */
    bool hasVertexShader() const;
    /**
     * @return True if this shader object has a fragment shader
     */
    bool hasFragmentShader() const;
    /**
     * @return True if this shader object has a geometry shader
     */
    bool hasGeometryShader() const;
    /**
     * Sets the pointer from which the View matrix should be loaded from
     * @param viewMat A pointer to the viewMatrix to be tracked
     * @note This pointer is likely provided by a Camera subclass
     */
    inline void setViewMatPtr(const glm::mat4  *_viewMat) { this->viewMat.matrixPtr = _viewMat; }
    /**
     * Sets the pointer from which the Projection matrix should be loaded from
     * @param projectionMat A pointer to the projectionMatrix to be tracked
     * @note This pointer is likely provided by the Visualisation object
     */
    inline void setProjectionMatPtr(const glm::mat4 *_projectionMat) { this->projectionMat.matrixPtr = _projectionMat; }
    /**
     * Sets the pointer from which the Model matrix should be loaded from
     * @param modelMat A pointer to the modelMatrix to be tracked
     * @note This pointer is likely provided by the Visualisation object
     */
    inline void setModelMatPtr(const glm::mat4 *_modelMat) { this->modelMat.matrixPtr = _modelMat; }
    /**
     * Overrides the model matrix (and all dependent matrices) until useProgram() is next called
     * This is a fast version that doesnt switch around bound shaders
     * @param modelMat Pointer to the overriding modelMat
     * @note This will only throw exceptions in Debug mode
     * @throws runtime_error When called whilst current shader is not active
     */
    void overrideModelMat(const glm::mat4 *modelMat);
    void setLightsBuffer(GLuint bufferBindingPoint) { addBuffer(LIGHT_UNIFORM_BLOCK_NAME, GL_UNIFORM_BUFFER, bufferBindingPoint); }
    /**
     * Sets the uniform buffer which should be bound to the material uniform buffer block
     * @param bufferBindingPoint GL binding point where buffer is bound
     * @return Returns true if the current shader has a detected material buffer block
     * @see addBuffer(const char *, const GLenum, const GLuint)
     */
    bool setMaterialBuffer(const GLuint bufferBindingPoint) { return addBuffer(MATERIAL_UNIFORM_BLOCK_NAME, GL_UNIFORM_BUFFER, bufferBindingPoint); }
    /**
     * Sets the uniform buffer which should be bound to the material uniform buffer block
     * This will not retain the shared_ptr, it's upto you to keep it alive
     * @param buffer The buffer to be used
     * @note Convenience method, implemented in BufferCore.cpp
     * @see addBuffer(const char *, const std::shared_ptr<BufferCore> &)
     */
    bool setMaterialBuffer(const std::shared_ptr<UniformBuffer> &buffer);
    /**
     * Updates the material index
     * This version is only to be called whilst the shader is active
     * @param materialIndex Index of the material to be used in the bound materials buffer
     * @see overrideMaterialID(unsigned int)
     */
    void setMaterialID(unsigned int materialIndex);
    /**
     * Updates the material index
     * This version is only to be called whilst the shader is active
     * This is a fast version that doesnt switch around bound shaders
     * @param materialIndex Index of the material to be used in the bound materials buffer
     * @note This will only throw exceptions in Debug mode
     * @throws runtime_error When called whilst current shader is not active
     * @see setMaterialId(unsigned int)
     */
    void overrideMaterialID(unsigned int materialIndex);
    /**
     * Sets the pointer which will apply a rotation to the ModelView matrix, rotating items rendered by this shader
     * @param _rotationPtr A pointer to the rotation will be tracked
     * @note Setting this pointer to nullptr will disable rotation
     * @note rotationPtr.xyz is the axis to rotate about
     * @note rotationPtr.w is the rotation angle in degrees
     */
    inline void setRotationPtr(glm::vec4 const *_rotationPtr) { this->rotationPtr = _rotationPtr; }
    /**
     * Sets the pointer which will apply a translation to the ModelView matrix, rotating items rendered by this shader
     * @param _translationPtr A pointer to the projectionMatrix to be tracked
     * @note Setting this pointer to nullptr will disable translation
     */
    inline void setTranslationPtr(glm::vec3 const *_translationPtr) { this->translationPtr = _translationPtr; }
    /**
     * Stores the details necessary for passing vertex position attributes to the shader via the modern method
     * @param vad The VertexAttributeDetail object containing the attribute data
     * @param update If true, the VAO will be rebuilt (not passing false when updating shared vbo id's will cause GL error's)
     */
    void setPositionsAttributeDetail(VertexAttributeDetail vad, bool update = true);
    /**
     * Stores the details necessary for passing vertex normal attributes
     * @param vad The VertexAttributeDetail object containing the attribute data
     * @param update If true, the VAO will be rebuilt (not passing false when updating shared vbo id's will cause GL error's)
     */
    void setNormalsAttributeDetail(VertexAttributeDetail vad, bool update = true);
    /**
     * Stores the details necessary for passing vertex color attributes to the shader
     * @param vad The VertexAttributeDetail object containing the attribute data
     * @param update If true, the VAO will be rebuilt (not passing false when updating shared vbo id's will cause GL error's)
     */
    void setColorsAttributeDetail(VertexAttributeDetail vad, bool update = true);
    /**
     * Stores the details necessary for passing vertex texture attributes to the shader
     * @param vad The VertexAttributeDetail object containing the attribute data
     * @param update If true, the VAO will be rebuilt (not passing false when updating shared vbo id's will cause GL error's)
     */
    void setTexCoordsAttributeDetail(VertexAttributeDetail vad, bool update = true);
    /**
     * Binds the named fragment shader output attribute to the specified framebuffer attachment point
     * @param attachmentPoint The GL_COLOR_ATTACHMENT index (likely in the range 0-7)
     * @param name the name of the data output as specified within the frag shader
     * @return Returns true if the re-linking of the shader was successful
     * @note If the framebuffer has one attachment, calling this is unnecessary.
     * @note Alternatively, you can hardcode attachment point 'n' in the shader, by preceding the output declaration with layout(location = n)
     * @note This function re-links the shader program, so may cause the shader to fail
     */
    bool setFragOutAttribute(GLuint attachmentPoint, const char *name);
    /**
     * Subclass of vertex attribute detail for generic vertex attributes
     * This attaches the name of the vertex attribute within the shader source
     */
    struct GenericVAD :public VertexAttributeDetail {
        GenericVAD(
            VertexAttributeDetail vad,
            const char* attributeName
            )
            : VertexAttributeDetail(vad)
            , attributeName(attributeName) {}
        /**
         * Identifier of the vertex attribute within the shader source
         */
        const char* attributeName;
    };
    /**
     * Holds additional information necessary for tracking generic vertex attributes
     * Key: Assigned generic buffer ID
     * Value: Other details necessary for binding
     */
    std::list<GenericVAD> gvads;
    /**
     * Holds generic vertex attributes that were not found within the shader
     * or went missing after a shader reload
     */
    std::list<GenericVAD> lostGvads;
    /**
     * Stores the details necessary for passing generic vertex attributes to the shader
     * @param attributeName The name of the attribute in the shader
     * @param vad The VertexAttributeDetail object containing the attribute data
     * @param update If true, the VAO will be rebuilt (not passing false when updating shared vbo id's will cause GL error's)
     */
    bool addGenericAttributeDetail(const char* attributeName, VertexAttributeDetail vad, bool update = true);

    inline void clearModelMatPtr() { this->modelMat.matrixPtr = nullptr; }
    inline void clearViewMatPtr() { this->viewMat.matrixPtr = nullptr; }
    inline void clearProjectionMatPtr() { this->projectionMat.matrixPtr = nullptr; }
    inline void clearPositionsAttributeDetail() {
        VertexAttributeDetail vad(GL_FLOAT, 3, sizeof(float));
        vad.location = this->positions.location;
        this->positions = vad;
    }
    inline void clearNormalsAttributeDetail() {
        VertexAttributeDetail vad(GL_FLOAT, NORMALS_SIZE, sizeof(float));
        vad.location = this->normals.location;
        this->normals = vad;
    }
    inline void clearColorsAttributeDetail() {
        VertexAttributeDetail vad(GL_FLOAT, 3, sizeof(float));
        vad.location = this->colors.location;
        this->colors = vad;
    }
    inline void clearTexCoordsAttributeDetail() {
        VertexAttributeDetail vad(GL_FLOAT, 2, sizeof(float));
        vad.location = this->texcoords.location;
        this->texcoords = vad;
    }
    bool removeGenericAttributeDetail(const char* attributeName);
    /**
     * Sets the face vbo to be added to the vertex array object
     * @param fbo The face vbo index
     */
    void setFaceVBO(GLuint fbo);
    /**
     * Sets the color uniform
     * @param color The RGB value of the color
     */
    void setColor(glm::vec3 color);
    /**
     * Sets the color uniform
     * @param color The RGBA value of the color
     */
    void setColor(glm::vec4 color);
    /**
     * Returns whether shader allows GL_BLEND to be used
     */
    bool supportsGL_BLEND() const { return supportsBlend; }

 private:
    /**
     * Whether shader allows usage of GL_BLEND
     * This is based on whether a fragment output includes an alpha channel
     */
    bool supportsBlend = false;
    /**
     * Face index vbo
     */
    GLuint fbo;
    /**
     * Vertex array object can be setup to carry all our vertex attributes
     * Therefore at render we are only required to bind the VAO, and not reconfigure each attribute
     */
    GLuint vao;
    /**
     * Configures the preexisting vao to contain vertex attribute arrays
     */
    void buildVAO();
    /**
     * Utility method for binding uniforms
     * @param rtn Pointer to store the uniform location in
     * @param uniformName The name of the uniform to locate
     * @param uniformType The type of uniform to check for
     * @note On failure rtn is set to -1
     */
    inline void bindUniform(int *rtn, const char *uniformName, GLenum uniformType) const;
    /**
     * Utility method for binding attributes
     * @param rtn Pointer to store the attribute location in
     * @param attributeName The name of the attribute to locate
     * @param attributeType1 The type of attribute to check for
     * @param attributeType2 The type of attribute to check for
     * @note On failure rtn is set to -1
     */
    inline void bindAttribute(int *rtn, const char *attributeName, GLenum attributeType1, GLenum attributeType2) const;
    /**
     * Disables the vertex attributes attached to this shader
     * @note Uses glDisableClientState() and glDisableVertexAttribArray()
     */
    void _clearProgram() override;
    /**
     * Updates shader dynamics
     * @note Called by ShaderCore::useProgram()
     */
    void _prepare() override;
    /**
     * Runs any shader config that MUST be called before shader use
     * @note Called by ShaderCore::useProgram()
     */
    void _useProgram() override;
    /**
     * Updates all matrix uniforms that contain the modelMat
     * @param force If passed this value overrides the stored modelMat.matrixPtr and translation/rotations
     * @note Called by ShaderCore::_prepare()
     */
    void _useProgramModelMatrices(const glm::mat4 *force = nullptr);
    /**
     * Compiles the shader sources specified at the objects creation
     * @param t_shaderProgram The shader program the shader should be attached to.
     * @note This function simply calls return compileShader()&&compileShader().. with each init list of source files
     */
    bool _compileShaders(const GLuint t_shaderProgram)override;
    /**
     * Binds Shaders class specific bindings to the shader program
     * These are primarily vertex attributes and modelview/projection matrices
     * Textures, buffers and misc uniforms are handled by ShaderCore::setupBindings() as these are also used by ComputeShader
     */
    void _setupBindings() override;
    /**
     * Information for binding the model matrix
     */
    UniformMatrixDetail modelMat;
    /**
     * Information for binding the view matrix
     */
    UniformMatrixDetail viewMat;
    /**
     * Information for binding the projection matrix
     */
    UniformMatrixDetail projectionMat;
    /**
     * Information for binding the material id
     */
    int materialIDLocation;
    int materialIDVal;
    /**
     * When positive this variable holds the location of the (combined) modelviewprojection matrix in the shader
     */
    int modelviewprojectionMatLoc;
    /**
     * When positive this vairable holds the location of the camera matrix in the shader
     * This is simply the modelview mat provided by the camera, before transformations are applied
     */
    int modelviewMatLoc;
    /**
     * When positive this vairable holds the location of the normal matrix in the shader
     */
    int normalMatLoc;
    /**
     * Cache's the previous frames modelview mat to be passed if _prevModelViewMat is required
     * @note Used for producing velocity map's
     */
    // glm::mat4 prevModelview;
    // int prevModelviewUniformLocation;
    /**
     * When !nullptr, points to a vector containing a rotation applied to the modelview matrix before binding
     */
    const glm::vec4 *rotationPtr;
    /**
    * When !nullptr, points to a vector containing a translation applied to the modelview matrix before binding
    */
    const glm::vec3 *translationPtr;
    /**
     * Holds information for binding the vertex positions attribute
     */
    VertexAttributeDetail positions;
    /**
    * Holds information for binding the vertex normals attribute
    */
    VertexAttributeDetail normals;
    /**
    * Holds information for binding the vertex colours attribute
    */
    VertexAttributeDetail colors;
    /**
    * Holds information for binding the vertex texture coordinates attribute
    */
    VertexAttributeDetail texcoords;
    /**
     * If the default color uniform identifier is found within the shader, it's location is stored here
     */
    int colorUniformLocation;
    /**
     * The number of components in the colour uniform that was located
     */
    int colorUniformSize;
    /**
     * The data to be passed to the colour uniform
     * This is static data, but we store the value so we can maintain it on reload
     */
    glm::vec4 colorUniformValue;
    /**
     * The color attachment point to bind to each output identifier within the frag shader
     * key: attachment point
     * value: output variable identifier
     */
    std::map<GLuint, std::string> fragShaderOutputLocations;
    /**
     * The path to the files which form the vertex shader
     * @note Used on shader reload
     */
    std::vector<std::string> *vertexShaderFiles;
    /**
    * The path to the files which form the fragment shader
    * @note Used on shader reload
    */
    std::vector<std::string> *fragmentShaderFiles;
    /**
    * The path to the files which form the geometry shader
    * @note Used on shader reload
    */
    std::vector<std::string> *geometryShaderFiles;
    /**
     * The GLSL version used within the vertex shader source
     * @note This value is detected from the #version define in the source file
     */
    int vertexShaderVersion;
    /**
    * The GLSL version used within the fragment shader source
    * @note This value is detected from the #version define in the source file
    */
    int fragmentShaderVersion;
    /**
    * The GLSL version used within the geometry shader source
    * @note This value is detected from the #version define in the source file
    */
    int geometryShaderVersion;
};

#endif  // SRC_SHADER_SHADERS_H_
