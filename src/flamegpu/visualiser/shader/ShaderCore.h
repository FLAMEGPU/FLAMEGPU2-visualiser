#ifndef SRC_FLAMEGPU_VISUALISER_SHADER_SHADERCORE_H_
#define SRC_FLAMEGPU_VISUALISER_SHADER_SHADERCORE_H_

#include <vector>
#include <map>
#include <list>
#include <memory>
#include <utility>
#include <string>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "flamegpu/visualiser/util/GLcheck.h"
#include "flamegpu/visualiser/interface/Reloadable.h"


namespace flamegpu {
namespace visualiser {

class BufferCore;  // Implementation of addBuffer(const char *, std::shared_ptr<BufferCore>) found in BufferCore.cpp
class Texture;  // Impementation of addTexture(const char *textureNameInShader, std::shared_ptr<Texture> texture) found in Texture.cpp
/**
 * This class is is a wrapper for the core OpenGL shader operations
 * Uniforms, textures and buffers can be automatically bound using the addXXX() methods, so they are provided to the shader
 * Subclasses may implement additional bindings
 * Bound items will then be automatically rebound when the shader is reloaded
 * Bound dynamic uniforms will automatically refresh the value everytime useProgram() is called
 *
 * This class is great for quickly using shaders, if you start double binding/unbinding stuff you might find bugs though
 *
 * @see Shaders
 * @see ComputeShader
 * @todo Replace uniform bindings pointers with weak ptrs
 */
class ShaderCore : public Reloadable {
 private:
    /**
     * OpenGLs reference to the shader
     * Negative values are invalid, meaning the shader is not ready
     */
    int programId;
    /**
     * Whether the shader object should exit program execution if a shader compilation/linking fails
     */
    static bool exitOnError;
    /**
     * A constant string to identify the particular shader instance
     */
    std::string shaderTag;

 protected:
    /**
     * Construtor, initialises common values
     * @note Does not setup the shader object (reload() does that)
     */
    ShaderCore();
    /**
    * Copy constructor
    */
    ShaderCore(const ShaderCore &other);
    /**
     * There is no reason to ever have a pointer to ShaderCore (the subclasses are too distinct, are used via their own methods)
     * Therefore no reason to delete one directly, hence protected
     */
    virtual ~ShaderCore();

 public:
    /**
     * Returns the shader tag, this is a rough identifier of the shader
     * This value is produced by concatenating the file name (sans extension) of the final file provided for each shader attatched to the object
     */
    inline const char* getShaderTag() const { return this->shaderTag.c_str(); }
    /**
     * Reloads the shader source from file, recompiles it and rebinds all bound items
     * @note It is expected that subclass constructors call this method after configuring their sources
     */
    void reload() final;
    /**
     * Returns OpenGLs reference to the shader
     * @return The OpenGL program id of the shader
     * @note Negative values mean the shader is not currently registered with OpenGL
     */
    int getProgram() const { return programId; }
    /**
    * Disables the currently active shader program and asks subclasses to clear any enabled client states or attribute arrays
    * @see _clearProgram()
    */
    void clearProgram();
    /**
     * Binds all dynamic uniforms/textures and subclass stuff such as vertex attribs
     * @param autoclear If true, the program will not remain bound after calling
     * @see _prepare()
    */
    void prepare(bool autoclear = true);
    /**
     * Calls glUseProgram(GLuint)
     * @param autoPrepare Automatically calls prepare() if true is passed
     */
    void useProgram(bool autoPrepare = true);
    /**
     * Returns whether the shader is compiled and ready
     * @return True if the shader is ready to be used, else false
     */
    bool isReady() const { return programId >= 0; }
    /**
     * Deletes the shader program, also calls clearProgram()
     * @see glDeleteProgram(GLuint)
     */
    void destroyProgram();
    /**
     * Remembers a pointer to an array of upto 4 floats that will be updated everytime useProgram() is called on this Shaders object
     * If a dynamic uniform with the same uniformName is already bound, it will be replaced
     * @param uniformName The name of the uniform within the shader
     * @param arry A pointer to the array of floats
     * @param count The number of floats provided in the array (a maximum of 4)
     * @returns false if the uniform name was not found or count is outside of the inclusive range 1-4
     * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
     * @see addDynamicUniform(const char *, const GLint *, unsigned int)
     */
    bool addDynamicUniform(const char *uniformName, const GLfloat *arry, unsigned int count = 1);
    /**
     * Remembers a pointer to an array of upto 4 integers that will be updated everytime useProgram() is called on this Shaders object
     * If a dynamic uniform with the same uniformName is already bound, it will be replaced
     * @param uniformName The name of the uniform within the shader
     * @param arry A pointer to the array of integers
     * @param count The number of integers provided in the array (a maximum of 4)
     * @returns false if the uniform name was not found or count is outside of the inclusive range 1-4
     * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
     * @see addDynamicUniform(const char *, const GLfloat *, unsigned int)
     */
    bool addDynamicUniform(const char *uniformName, const GLint *arry, unsigned int count = 1);
    /**
     * Remembers a pointer to an array of upto 4 unsigned integers that will be updated everytime useProgram() is called on this Shaders object
     * If a dynamic uniform with the same uniformName is already bound, it will be replaced
     * @param uniformName The name of the uniform within the shader
     * @param arry A pointer to the array of unsigned integers
     * @param count The number of unsigned integers provided in the array (a maximum of 4)
     * @returns false if the uniform name was not found or count is outside of the inclusive range 1-4
     * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
     * @see addDynamicUniform(const char *, const GLfloat *, unsigned int)
     */
    bool addDynamicUniform(const char *uniformName, const GLuint *arry, unsigned int count = 1);
    /**
     * Remembers a pointer to an array of upto 4 doubles that will be updated everytime useProgram() is called on this Shaders object
     * If a dynamic uniform with the same uniformName is already bound, it will be replaced
     * @param uniformName The name of the uniform within the shader
     * @param arry A pointer to the array of doubles
     * @param count The number of doubles provided in the array (a maximum of 4)
     * @returns false if the uniform name was not found or count is outside of the inclusive range 1-4
     * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
     * @see addDynamicUniform(const char *, const GLint *, unsigned int)
     */
    bool addDynamicUniform(const char *uniformName, const GLdouble *arry, unsigned int count = 1);
    /**
    * Remembers a pointer to a mat4 that will be updated everytime useProgram() is called on this Shaders object
    * If a dynamic uniform with the same uniformName is already bound, it will be replaced
    * @param uniformName The name of the uniform within the shader
    * @param mat Pointer to the mat4
    * @returns false if the uniform name was not found
    * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
    * @see addDynamicUniform(const char *, const GLfloat *, unsigned int)
    */
    bool addDynamicUniform(const char *uniformName, const glm::mat4 *mat);
    /**
     * Sets the value of a vector of upto 4 floats within the shader
     * If a static uniform with the same uniformName is already bound, it will be replaced
     * @param uniformName The name of the uniform within the shader
     * @param arry A pointer to the array of floats
     * @param count The number of floats provided in the array (a maximum of 4)
     * @returns false if the uniform name was not found or count is outside of the inclusive range 1-4
     * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
     */
    bool addStaticUniform(const char *uniformName, const GLfloat *arry, unsigned int count = 1);
    /**
     * Sets the value of a vector of upto 4 ints within the shader
     * If a static uniform with the same uniformName is already bound, it will be replaced
     * @param uniformName The name of the uniform within the shader
     * @param arry A pointer to the array of integers
     * @param count The number of integers provided in the array (a maximum of 4)
     * @returns false if the uniform name was not found or count is outside of the inclusive range 1-4
     * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
     */
    bool addStaticUniform(const char *uniformName, const GLint *arry, unsigned int count = 1);
    /**
    * Sets the value of a vector of upto 4 unsigned ints within the shader
    * If a static uniform with the same uniformName is already bound, it will be replaced
    * @param uniformName The name of the uniform within the shader
    * @param arry A pointer to the array of unsigned integers
    * @param count The number of unsigned integers provided in the array (a maximum of 4)
    * @returns false if the uniform name was not found or count is outside of the inclusive range 1-4
    * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
    */
    bool addStaticUniform(const char *uniformName, const GLuint *arry, unsigned int count = 1);
    /**
     * Sets the value of a vector of upto 4 doubles within the shader
     * If a static uniform with the same uniformName is already bound, it will be replaced
     * @param uniformName The name of the uniform within the shader
     * @param arry A pointer to the array of doubles
     * @param count The number of doubles provided in the array (a maximum of 4)
     * @returns false if the uniform name was not found or count is outside of the inclusive range 1-4
     * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
     */
    bool addStaticUniform(const char *uniformName, const GLdouble *arry, unsigned int count = 1);
    /**
    * Sets the value of a mat4 within the shader
    * If a static uniform with the same uniformName is already bound, it will be replaced
    * @param uniformName The name of the uniform within the shader
    * @param mat Pointer to the mat4
    * @returns false if the uniform name was not found
    * @note Even when false is returned, the value will be stored for reprocessing on shader reloads
    * @see addDynamicUniform(const char *, const GLfloat *, unsigned int)
    */
    bool addStaticUniform(const char *uniformName, const glm::mat4 *mat);
    /**
     * Binds a texture to be loaded when useProgram() is called
     * If a uniform with the same uniformName as textureNameInShader is already bound, it will be replaced
     * @param textureNameInShader The name of the uniform within the shader this texture should be bound to
     * @param type The type of texture being bound (e.g. GL_TEXTURE_2D)
     * @param textureName The name of the texture (as returned by glGenTexture())
     * @param textureUnit The texture unit assigned to the texture (as set via glActiveTexture() glBindTexture())
     * @return The texture unit the texture has been bound to, on failure (due to no texture units remaining) -1
     * @note Texture unit bindings are not shader specific, they persist between shader calls
     * @note Different texture type can be bound to the same unit, the sampler type used in shader selects the correct texture
     */
    bool addTexture(const char *textureNameInShader, GLenum type, GLint textureName, GLuint textureUnit);
    /**
     * Attaches the specified buffer to the shader if bufferNameInShader can be found
     * If a uniform with the same uniformName as textureNameInShader is already bound, it will be replaced
     *This will not retain the shared_ptr, it's upto you to keep it alive
     * @param textureNameInShader The indentifier of the buffer within the shader source
     * @param texture The texture to be used
     * @note Convenience method, implemented in Texture.cpp
     */
    bool addTexture(const char *textureNameInShader, const std::shared_ptr<const Texture> &texture);
    /**
     * Attaches the specified buffer to the shader if bufferNameInShader can be found
     * If a buffer with the same bufferNameInShader is already bound, it will be replaced
     * This is for 'program resources', NOT texture buffers
     * @param bufferNameInShader The indentifier of the buffer within the shader source
     * @param bufferType The type of buffer (probably GL_SHADER_STORAGE_BUFFER or GL_UNIFORM_BUFFER)
     * @param bufferBindingPoint The buffer's bind point
     * @return True if buffer name is found in the current shader to be bound
     */
    bool addBuffer(const char *bufferNameInShader, GLenum bufferType, GLuint bufferBindingPoint);
    /**
     * Attaches the specified buffer to the shader if bufferNameInShader can be found
     * If a buffer with the same bufferNameInShader is already bound, it will be replaced
     * This is for 'program resources', NOT texture buffers
     * This will not retain the shared_ptr, it's upto you to keep it alive
     * @param bufferNameInShader The identifier of the buffer within the shader source
     * @param buffer The buffer to be used
     * @return True if buffer name is found in the current shader to be bound
     * @note Convenience method, implemented in BufferCore.cpp
     */
    bool addBuffer(const char *bufferNameInShader, const std::shared_ptr<BufferCore> &buffer);
    /**
     * Unbinds the named dynamic uniform
     * @note This will not replace the value cached in the shader, reload() is necessary to achieve that
     */
    bool removeDynamicUniform(const char *uniformName);
    /**
     * Unbinds the named static uniform
     * @note This will not replace the value cached in the shader, reload() is necessary to achieve that
     * @note This will also remove texture uniforms of the same name
     */
    bool removeStaticUniform(const char *uniformName);
    /**
     * Unbinds the named texture uniform
     * @note This will also remove static uniforms of the same name
     */
    bool removeTextureUniform(const char *uniformName);
    /**
     * Unbinds the named buffer uniform
     * @note This will not replace the value cached in the shader, reload() is necessary to achieve that
     */
    bool removeBuffer(const char *nameInShader);
    /**
     * Sets the static setting determining whether the program should exit on a shader compilation failure
     */
    static void setExitOnError(const bool _exitOnError) { ShaderCore::exitOnError = _exitOnError; }
    /**
     * Returns the status of the static setting determining whether the program should exit on shader compilation failure
     * @return True if the program should exit on shader compilation failure
     */
    static bool getExitOnError() { return ShaderCore::exitOnError; }
    /**
     * Attempts to locate the specified uniform's location and type within the provided shader
     * @param uniformName The name of the uniform
     * @param shaderProgram The programId of the shader
     * @return A pair object whereby the first item is the uniform'd location, and the second item is the type. On failure the first item will be -1
     * @note Type can be any enum from: GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4, GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4, GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT4, GL_SAMPLER_2D, or GL_SAMPLER_CUBE
     */
    static std::pair<int, GLenum> findUniform(const char *uniformName, const int shaderProgram);
    /**
     * Attempts to locate the specified attribute's location and type
     * @param attributeName The name of the attribute
     * @param shaderProgram The programId of the shaderprogram
     * @return A pair object whereby the first item is the attribute's location, and the second item is the type. On failure the first item will be -1
     * @note Type can be any enum from: GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4, GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4, GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT4, GL_SAMPLER_2D, or GL_SAMPLER_CUBE
     */
    static std::pair<int, GLenum> findAttribute(const char *attributeName, const int shaderProgram);

 private:
    static GLenum getResourceBlock(GLenum bufferType);
    /**
     * Holds shaders thats have been compiled, so that they can be deleted
     * @see deleteShaders()
     */
    std::vector<GLuint> floatingShaders;
    /**
     * Simply wraps glCreateShader(GLenum), so that we can track created shaders to clean them after linking
     */
    GLuint createShader(GLenum type) {
        GLuint t = GL_CALL(glCreateShader(type));
        floatingShaders.push_back(t);
        return t;
    }
    /**
     * Simply calls glDeleteShader(GLuint) on all values stored in floatingShaders and then clears floatingShaders
     * Once they have been linked to the program, they are nolonger required
     */
    void deleteShaders() {
        for (auto i : floatingShaders) {
            GL_CALL(glDeleteShader(i));
        }
        floatingShaders.clear();
    }
    /**
     * Holds all the necessary information for binding dynamic uniform int and float vectors
     */
    struct DynamicUniformDetail {
        /**
         * GL_INT, GL_UNSIGNED_INT GL_FLOAT
         */
        const GLenum type;
        /**
         * Pointer to the data we want to copy to the shader uniform
         */
        const void *data;
        /**
         * Number of vector components
         */
        const unsigned int count;
        /**
         * Identifier of the uniform within the shader source
         */
        const std::string uniformName;
    };
    /**
     * Remembers a pointer to an array of upto 4 integers that will be updated everytime useProgram() is called on this Shaders object
     * @see addDynamicUniform(const char *, const GLfloat *, unsigned int)
     * @see addDynamicUniform(const char *, const GLint *, unsigned int)
     */
    bool addDynamicUniform(DynamicUniformDetail d);
    /**
     * Holds dynamic uniforms that have been found within the shader
     * Key: Uniform location within shader
     * Val: Additional information necessary for setting the uniform
     */
    std::map<GLint, DynamicUniformDetail> dynamicUniforms;
    /**
     * Holds dynamic uniforms that were not found within the shader
     */
    std::list<DynamicUniformDetail> lostDynamicUniforms;
    /**
     * Holds all the necessary information for binding static uniform int and float vectors
     */
    struct StaticUniformDetail {
        /**
         * GL_INT, GL_UNSIGNED_INT or GL_FLOAT
         */
        const GLenum type;
        /**
         * Data to be stored in the shader uniform
         * @note components greater than count-1 are likely to contain garbage data
         * @note this may actually contain a glm::vec4 if type == GL_FLOAT, glm::ivec4 etc
         */
        const glm::mat4 data;
        /**
         * Number of vector components
         */
        const unsigned int count;
        /**
         * Identifier of the uniform within the shader source
         */
        const std::string uniformName;
    };
    /**
     * Holds information necessary for setting the static uniforms to be bound to the shader
     */
    std::list<StaticUniformDetail> staticUniforms;
    /**
     * Holds all the additional information for binding texture uniforms
     */
    struct UniformTextureDetail {
        /**
         * The name of the texture (as returned by glGenTexture())
         */
        const GLuint name;
        /**
         * The type of texture (e.g. GL_TEXTURE2D)
         */
        const GLenum type;
    };
    /**
     * Holds additional information necessary for tracking texture uniforms
     * Items in this vector have a counterpart in staticUniforms
     * @see staticUniforms
     * Key: The tex buffer we're storing this in (shader specific)
     * Value: Associated information
     */
    std::map<GLint, UniformTextureDetail> textures;
    /**
     * Holds all the additional information for binding texture uniforms
     */
    struct BufferDetail {
        /**
         * Name of the buffer within the shader
         */
        const std::string nameInShader;
        /**
         * The type of buffer (e.g. GL_SHADER_STORAGE_BUFFER, GL_UNIFORM_BUFFER)
         */
        const GLenum type;
        /**
         * The binding point of the buffer (as set with glBindBufferBase())
         */
        const GLuint bindingPoint;
    };
    /**
     * Holds additional information necessary for tracking buffers
     */
    std::map<GLuint, BufferDetail> buffers;
    /**
     * Holds buffers that were not found within the shader
     * or went missing after a shader reload
     */
    std::list<BufferDetail> lostBuffers;
    /**
     * Subclasses should use this to clear any enabled client states or attribute arrays
     * If not overriden, does nothing
     * Called by clearProgram()
     */
    virtual void _clearProgram() {}
    /**
     * Subclasses should use this to apply any subclass specific shader bindings
     * If not overriden, does nothing
     * @note Called by prepare()
     */
    virtual void _prepare() {}
    /**
     *Subclasses should use this to config shader specifics that MUST be executed before shader use
     * @note Called by useProgram()
     */
    virtual void _useProgram() {}
    /**
     * Subclasses should use this to call compileShader() with each shader src
     * @param t_shaderProgram Temporary shader program ID which succesfully compiled shaders should be attatched to
     * Called by reload()
     */
    virtual bool _compileShaders(const GLuint t_shaderProgram) = 0;
    /**
     * Locates all subclass specific bound uniforms, attributes, textures and storage within the shader
     * Called by setupBindings()
     */
    virtual void _setupBindings() {}

 protected:
    /**
     * Locates all bound uniforms, attributes, textures and storage within the shader
     * Calls _setupBindings()
     */
    void setupBindings();
    /**
     * Compiles the specified shader source and attatches it to the provided program
     * @param t_shaderProgram The shader program to attach the compiled shader to
     * @param type GL_VERTEX_SHADER/GL_FRAGMENT_SHADER/GL_GEOMETRY_SHADER/GL_COMPUTE_SHADER
     * @param shaderSourceFiles An initialiser list ({a, b, c}) of paths to shader sources
     * @param extension Raw shader code to be appended to the shader sources
     * @return The shader version detected, -1 on compilation failure
     */
    int compileShader(const GLuint t_shaderProgram, GLenum type, std::vector<std::string> *shaderSourceFiles, const std::string& extension = "");
    /**
     * Loads the text from the provided filepath
     * @return A pointer to the loaded shader source
     * @note the returned pointer is allocated via malloc, and should be free'd when nolonger required
     */
    static char *loadShaderSource(const char *file);
    /**
     * Looks for the '#version xx' tag in the provided shader source and returns the numeric value
     * @param shaderSources The shader code to detect the version from
     * @return The detected shader version, 0 if one was not found
     */
    static unsigned int findShaderVersion(std::vector<const char*> const & shaderSources);
    /**
     * Copies the init list to a std::vector of std:strings on the heap
     * @note You should delete the ptr returned by this yourself
     */
    static std::vector<std::string> *buildFileVector(std::initializer_list<std::string>);
    /**
     * Checks whether the specified shader program linked succesfully.
     * Linking errors are printed to stderr and compileSuccessflag is set to false on failure.
     * @param programId Location of the shader program to check
     * @return True if no errors were detected
     * @note For some reason program compilation failure logs don't seem to work (the same as shader compilation)
     */
    bool checkProgramLinkError(const GLuint programId) const;

 private:
    /**
     * Checks whether the specified shader compiled succesfully.
     * Compilation errors are printed to stderr and compileSuccessflag is set to false on failure.
     * @param shaderId Location of the shader to check
     * @param shaderPath Path to the shader being checked (so that it can be easily identified in the error log)
     * @return True if no errors were detected
     */
    static bool checkShaderCompileError(const GLuint shaderId, const char *shaderPath);
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_SHADER_SHADERCORE_H_
