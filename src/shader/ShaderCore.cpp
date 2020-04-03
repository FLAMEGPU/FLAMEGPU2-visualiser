#include "shader/ShaderCore.h"
#include <cstdlib>  // < _splitpath() Windows only, need to rewrite linux ver
#include <regex>
#include "util/warnings.h"
DISABLE_WARNING_PUSH
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNING_POP
#include "util/StringUtils.h"
#include "shader/Shaders.h"
#include "util/Resources.h"

bool ShaderCore::exitOnError = false;  // Tempted to use pre-processor macros to swap this default to true on release mode
// Constructors/Destructors
ShaderCore::ShaderCore()
    : programId(-1), shaderTag(const_cast<char *>("")) { }
ShaderCore::ShaderCore(const ShaderCore &other)
    : ShaderCore() {
    // Copy across all member variables: e.g uniforms, textures, buffers etc
    // floatingShaders;  // This is managed by compile, it's more of a temporary data structure
    // dynamicUniforms, lostDynamicUniforms
    for (const auto &i : other.dynamicUniforms)
        this->lostDynamicUniforms.push_back(DynamicUniformDetail(i.second));
    for (const auto &i : other.lostDynamicUniforms)
        this->lostDynamicUniforms.push_back(DynamicUniformDetail(i));
    // staticUniforms
    for (const auto &i : other.staticUniforms)
        this->staticUniforms.push_back(StaticUniformDetail(i));
    // textures
    for (const auto &i : other.textures)
        this->textures.insert({ i.first, UniformTextureDetail(i.second) });
    // buffers, lostBuffers
    for (const auto &i : other.buffers)
        this->lostBuffers.push_back(BufferDetail(i.second));
    for (const auto &i : other.lostBuffers)
        this->lostBuffers.push_back(BufferDetail(i));
}
ShaderCore::~ShaderCore() {
    if (this->shaderTag[0] != '\0') delete[] this->shaderTag;
}
// Core
void ShaderCore::reload() {
    GL_CHECK();
    // Clear shadertag
    if (this->shaderTag[0] != '\0') delete[] this->shaderTag;
    this->shaderTag = const_cast<char *>("");
    while (true) {  // Iterate until shader compilation has been corrected
        // Create temporary shader program
        GLuint t_programId = GL_CALL(glCreateProgram());
        // Pass it to subclass to compile shaders
        if (this->_compileShaders(t_programId)) {
            //  Link the program and ensure the program compiled correctly;
            GL_CALL(glLinkProgram(t_programId));

            //  If the program linked ok, then we update the instance variable (for live reloading)
            if (this->checkProgramLinkError(t_programId)) {
                //  Destroy the old program
                this->destroyProgram();
                //  Update the class var for the next usage.
                this->programId = t_programId;
            } else {
                // Compilation failed, cleanup temp program
                GL_CALL(glDeleteProgram(t_programId));
                deleteShaders();
                fprintf(stderr, "Press any key to recompile.\n");
                getchar();
                continue;
            }
        } else {
            // Compilation failed, cleanup temp program
            GL_CALL(glDeleteProgram(t_programId));
            deleteShaders();
            fprintf(stderr, "Press any key to recompile.\n");
            getchar();
            continue;
        }
        break;
    }
    this->setupBindings();
}
void ShaderCore::setupBindings() {
    // Refresh dynamic uniforms
    std::list<DynamicUniformDetail> t_dynamicUniforms;
    t_dynamicUniforms.splice(t_dynamicUniforms.end(), lostDynamicUniforms);
    for (std::map<GLint, DynamicUniformDetail>::iterator i = dynamicUniforms.begin(); i != dynamicUniforms.end(); ++i) {
        t_dynamicUniforms.push_back(i->second);
    }
    dynamicUniforms.clear();
    for (DynamicUniformDetail const &d : t_dynamicUniforms) {
        GLint location = GL_CALL(glGetUniformLocation(this->programId, d.uniformName));
        if (location != -1) {
            dynamicUniforms.emplace(location, d);
        } else {  // If the buffer isn't found, remind the user
            lostDynamicUniforms.push_front(d);
            printf("%s: Dynamic uniform '%s' could not be located on shader reload.\n", this->shaderTag, d.uniformName);
        }
    }
    // Refresh static uniforms
    GL_CALL(glUseProgram(this->programId));
    for (std::list<StaticUniformDetail>::iterator i = staticUniforms.begin(); i != staticUniforms.end(); ++i) {
        GLint location = GL_CALL(glGetUniformLocation(this->programId, i->uniformName));
        if (location != -1) {
            if (i->type == GL_FLOAT) {
                static_assert(sizeof(int) == sizeof(float), "Error: int and float sizes differ, static float uniforms may be corrupted.\n");
                if (i->count == 1) {
                    GL_CALL(glUniform1fv(location, 1, reinterpret_cast<const GLfloat *>(glm::value_ptr(i->data))));
                } else if (i->count == 2) {
                    GL_CALL(glUniform2fv(location, 1, reinterpret_cast<const GLfloat *>(glm::value_ptr(i->data))));
                } else if (i->count == 3) {
                    GL_CALL(glUniform3fv(location, 1, reinterpret_cast<const GLfloat *>(glm::value_ptr(i->data))));
                } else if (i->count == 4) {
                    GL_CALL(glUniform4fv(location, 1, reinterpret_cast<const GLfloat *>(glm::value_ptr(i->data))));
                }
            } else if (i->type == GL_INT) {
                if (i->count == 1) {
                    GL_CALL(glUniform1iv(location, 1, glm::value_ptr(i->data)));
                } else if (i->count == 2) {
                    GL_CALL(glUniform2iv(location, 1, glm::value_ptr(i->data)));
                } else if (i->count == 3) {
                    GL_CALL(glUniform3iv(location, 1, glm::value_ptr(i->data)));
                } else if (i->count == 4) {
                    GL_CALL(glUniform4iv(location, 1, glm::value_ptr(i->data)));
                }
            } else if (i->type == GL_UNSIGNED_INT) {
                if (i->count == 1) {
                    GL_CALL(glUniform1uiv(location, 1, reinterpret_cast<const GLuint *>(glm::value_ptr(i->data))));
                } else if (i->count == 2) {
                    GL_CALL(glUniform2uiv(location, 1, reinterpret_cast<const GLuint *>(glm::value_ptr(i->data))));
                } else if (i->count == 3) {
                    GL_CALL(glUniform3uiv(location, 1, reinterpret_cast<const GLuint *>(glm::value_ptr(i->data))));
                } else if (i->count == 4) {
                    GL_CALL(glUniform4uiv(location, 1, reinterpret_cast<const GLuint *>(glm::value_ptr(i->data))));
                }
            } else if (i->type == GL_FLOAT_MAT4) {
                GL_CALL(glUniformMatrix4fv(location, 1, false, reinterpret_cast<const GLfloat *>(glm::value_ptr(i->data))));
            }
        } else {  // If the uniform isn't found again, remind the user
            printf("%s: Static uniform '%s' could not located on shader reload.\n", this->shaderTag, i->uniformName);
        }
    }
    // Refresh buffers
    std::list<BufferDetail> t_buffers;
    t_buffers.splice(t_buffers.end(), lostBuffers);
    for (std::map<GLuint, BufferDetail>::iterator i = buffers.begin(); i != buffers.end(); ++i) {
        t_buffers.push_back(i->second);
    }
    buffers.clear();
    for (BufferDetail const &d : t_buffers) {  // Replace d.type with GL_SHADER_STORAGE_BLOCK or ? GL_BUFFER_VARIABLE
        GLenum blockType = getResourceBlock(d.type);
        if (blockType == GL_INVALID_ENUM)
            continue;
        GLuint uniformBlockIndex = GL_CALL(glGetProgramResourceIndex(this->programId, blockType, d.nameInShader));
        if (uniformBlockIndex != GL_INVALID_INDEX) {
            auto rtn = buffers.emplace(uniformBlockIndex, d);
            if (!rtn.second)fprintf(stderr, "Somehow a buffer was bound twice.");
            GL_CALL(glUniformBlockBinding(this->programId, uniformBlockIndex, d.bindingPoint));
        } else if (strcmp(d.nameInShader, Shaders::LIGHT_UNIFORM_BLOCK_NAME) && strcmp(d.nameInShader, Shaders::MATERIAL_UNIFORM_BLOCK_NAME)) {  // If the buffer isn't found, remind the user, Don't warn for known system bufferS
            lostBuffers.push_front(d);
            printf("%s: Buffer '%s' could not be located on shader reload.\n", this->shaderTag, d.nameInShader);
        }
    }
    // Refresh subclass specific bindings
    this->_setupBindings();
    GL_CALL(glUseProgram(0));
}
void ShaderCore::prepare(bool autoClear) {
    // Kill if shader isn't built
    if (this->programId <= 0) {
        return;
    }
    GL_CALL(glUseProgram(this->programId));

#ifdef _DEBUG
    {  // Debug verification that textures are bound as expected
        GLint whichID = 0;
        for (auto utd : textures) {
            GL_CALL(glActiveTexture(GL_TEXTURE0 + utd.first));
            if (utd.second.type == GL_TEXTURE_2D) {
                GL_CALL(glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichID));
            } else if (utd.second.type == GL_TEXTURE_CUBE_MAP) {
                GL_CALL(glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &whichID));
            } else if (utd.second.type == GL_TEXTURE_BUFFER) {
                GL_CALL(glGetIntegerv(GL_TEXTURE_BINDING_BUFFER, &whichID));
            } else if (utd.second.type == GL_TEXTURE_2D_MULTISAMPLE) {
                GL_CALL(glGetIntegerv(GL_TEXTURE_BINDING_2D_MULTISAMPLE, &whichID));
            } else {
                fprintf(stderr, "Unexpected texture type when verifying texture unit bindings, please add to ShaderCore::useProgram()\n");
                continue;
            }
            // Updated textures use unique buffers, if for some reason this fails we have exceeded GL_MAX_COMBINED_TEXTURE_UNITS
            // and must start reusing texture units (for the given texture type) based on a static flag/counter
            assert(static_cast<GLuint>(whichID) == utd.second.name);
        }
        // Reset to texture unit 0 for doing work
        GL_CALL(glActiveTexture(GL_TEXTURE0));
    }
#endif

    // Set any dynamic uniforms
    for (std::map<GLint, DynamicUniformDetail>::iterator i = dynamicUniforms.begin(); i != dynamicUniforms.end(); ++i) {
        if (i->second.type == GL_FLOAT) {
            if (i->second.count == 1) {
                GL_CALL(glUniform1fv(i->first, 1, reinterpret_cast<const GLfloat *>(i->second.data)));
            } else if (i->second.count == 2) {
                GL_CALL(glUniform2fv(i->first, 1, reinterpret_cast<const GLfloat *>(i->second.data)));
            } else if (i->second.count == 3) {
                GL_CALL(glUniform3fv(i->first, 1, reinterpret_cast<const GLfloat *>(i->second.data)));
            } else if (i->second.count == 4) {
                GL_CALL(glUniform4fv(i->first, 1, reinterpret_cast<const GLfloat *>(i->second.data)));
            }
        } else if (i->second.type == GL_INT) {
            if (i->second.count == 1) {
                GL_CALL(glUniform1iv(i->first, 1, reinterpret_cast<const GLint *>(i->second.data)));
            } else if (i->second.count == 2) {
                GL_CALL(glUniform2iv(i->first, 1, reinterpret_cast<const GLint *>(i->second.data)));
            } else if (i->second.count == 3) {
                GL_CALL(glUniform3iv(i->first, 1, reinterpret_cast<const GLint *>(i->second.data)));
            } else if (i->second.count == 4) {
                GL_CALL(glUniform4iv(i->first, 1, reinterpret_cast<const GLint *>(i->second.data)));
            }
        } else if (i->second.type == GL_UNSIGNED_INT) {
            if (i->second.count == 1) {
                GL_CALL(glUniform1uiv(i->first, 1, reinterpret_cast<const GLuint *>(i->second.data)));
            } else if (i->second.count == 2) {
                GL_CALL(glUniform2uiv(i->first, 1, reinterpret_cast<const GLuint *>(i->second.data)));
            } else if (i->second.count == 3) {
                GL_CALL(glUniform3uiv(i->first, 1, reinterpret_cast<const GLuint *>(i->second.data)));
            } else if (i->second.count == 4) {
                GL_CALL(glUniform4uiv(i->first, 1, reinterpret_cast<const GLuint *>(i->second.data)));
            }
        } else if (i->second.type == GL_FLOAT_MAT4) {
            GL_CALL(glUniformMatrix4fv(i->first, 1, false, reinterpret_cast<const GLfloat *>(i->second.data)));
        }
    }
    // Set any subclass specific stuff
    this->_prepare();

    if (autoClear) {
        GL_CALL(glUseProgram(0));
    }
}
void ShaderCore::useProgram(bool autoPrepare) {
    // Kill if shader isn't built
    if (this->programId <= 0) {
            return;
    }

    if (autoPrepare)
        this->prepare(false);
    else
        GL_CALL(glUseProgram(this->programId));

    // Is this required with new tex?
    // // Set any Texture buffers
    // for (auto utd : textures)
    // {  // Why textures only here?
    //     glActiveTexture(GL_TEXTURE0 + utd.first);
    //     glBindTexture(utd.second.type, utd.second.name);
    // }

    this->_useProgram();
}
void ShaderCore::clearProgram() {
    this->_clearProgram();
    GL_CALL(glUseProgram(0));
}
void ShaderCore::destroyProgram() {
    if (this->programId > 0) {
        this->clearProgram();
        GL_CALL(glDeleteProgram(this->programId));
        this->programId = -1;
    }
}
// Bindings
bool ShaderCore::addDynamicUniform(const char *uniformName, const GLint *arry, unsigned int count) {
    return addDynamicUniform({ GL_INT, reinterpret_cast<const void*>(arry), count, uniformName });
}
bool ShaderCore::addDynamicUniform(const char *uniformName, const GLuint *arry, unsigned int count) {
    return addDynamicUniform({ GL_UNSIGNED_INT, reinterpret_cast<const void*>(arry), count, uniformName });
}
bool ShaderCore::addDynamicUniform(const char *uniformName, const GLfloat *arry, unsigned int count) {
    return addDynamicUniform({ GL_FLOAT, reinterpret_cast<const void*>(arry), count, uniformName });
}
bool ShaderCore::addDynamicUniform(const char *uniformName, const glm::mat4 *mat) {
    return addDynamicUniform({ GL_FLOAT_MAT4, reinterpret_cast<const void*>(mat), 1, uniformName });
}
bool ShaderCore::addDynamicUniform(DynamicUniformDetail d) {
    if (d.count > 0 && d.count <= 4) {
        // Purge any existing dynamic uniform which matches
        removeDynamicUniform(d.uniformName);
        if (this->programId > 0) {
            GLint location = GL_CALL(glGetUniformLocation(this->programId, d.uniformName));
            if (location != -1) {
                // Replace with new one
                // Can't use[] assignment constructor due to const elements
                dynamicUniforms.erase(location);
                dynamicUniforms.emplace(location, d);
                return true;
            }
            fprintf(stderr, "%s: Dynamic uniform named: %s was not found.\n", shaderTag, d.uniformName);
        }
        lostDynamicUniforms.push_back(d);
    }
    return false;
}
bool ShaderCore::addStaticUniform(const char *uniformName, const GLfloat *arry, unsigned int count) {
    // Purge any existing buffer which matches
    removeStaticUniform(uniformName);
    // Note we reinterpret_cast the data to from float to int
    staticUniforms.push_front({ GL_FLOAT, *reinterpret_cast<const glm::ivec4*>(arry), count, uniformName });
    if (this->programId > 0 && count > 0 && count <= 4) {
        GLint location = GL_CALL(glGetUniformLocation(this->programId, uniformName));
        if (location != -1) {
            GL_CALL(glUseProgram(this->programId));
            if (count == 1) {
                GL_CALL(glUniform1fv(location, 1, arry));
            } else if (count == 2) {
                GL_CALL(glUniform2fv(location, 1, arry));
            } else if (count == 3) {
                GL_CALL(glUniform3fv(location, 1, arry));
            } else if (count == 4) {
                GL_CALL(glUniform4fv(location, 1, arry));
            }
            GL_CALL(glUseProgram(0));
            return true;
        } else {
            fprintf(stderr, "%s: Static uniform named: %s was not found.\n", shaderTag, uniformName);
        }
    }
    return false;
}
bool ShaderCore::addStaticUniform(const char *uniformName, const GLint *arry, unsigned int count) {
    // Purge any existing buffer which matches
    removeStaticUniform(uniformName);
    staticUniforms.push_front({ GL_INT, *reinterpret_cast<const glm::ivec4*>(arry), count, uniformName });
    if (this->programId > 0 && count > 0 && count <= 4) {
        GLint location = GL_CALL(glGetUniformLocation(this->programId, uniformName));
        if (location != -1) {
            GL_CALL(glUseProgram(this->programId));
            if (count == 1) {
                GL_CALL(glUniform1iv(location, 1, arry));
            } else if (count == 2) {
                GL_CALL(glUniform2iv(location, 1, arry));
            } else if (count == 3) {
                GL_CALL(glUniform3iv(location, 1, arry));
            } else if (count == 4) {
                GL_CALL(glUniform4iv(location, 1, arry));
            }
            GL_CALL(glUseProgram(0));
            return true;
        } else {
            fprintf(stderr, "%s: Static uniform named: %s was not found.\n", shaderTag, uniformName);
        }
    }
    return false;
}
bool ShaderCore::addStaticUniform(const char *uniformName, const GLuint *arry, unsigned int count) {
    // Purge any existing buffer which matches
    removeStaticUniform(uniformName);
    staticUniforms.push_front({ GL_UNSIGNED_INT, *reinterpret_cast<const glm::ivec4*>(arry), count, uniformName });
    if (this->programId > 0 && count > 0 && count <= 4) {
        GLint location = GL_CALL(glGetUniformLocation(this->programId, uniformName));
        if (location != -1) {
            GL_CALL(glUseProgram(this->programId));
            if (count == 1) {
                GL_CALL(glUniform1uiv(location, 1, arry));
            } else if (count == 2) {
                GL_CALL(glUniform2uiv(location, 1, arry));
            } else if (count == 3) {
                GL_CALL(glUniform3uiv(location, 1, arry));
            } else if (count == 4) {
                GL_CALL(glUniform4uiv(location, 1, arry));
            }
            GL_CALL(glUseProgram(0));
            return true;
        } else {
            fprintf(stderr, "%s: Static uniform named: %s was not found.\n", shaderTag, uniformName);
        }
    }
    return false;
}
bool ShaderCore::addStaticUniform(const char *uniformName, const glm::mat4 *mat) {
    // Purge any existing buffer which matches
    removeStaticUniform(uniformName);
    staticUniforms.push_front({ GL_FLOAT_MAT4, *reinterpret_cast<const glm::ivec4*>(mat), 1, uniformName });
    if (this->programId > 0) {
        GLint location = GL_CALL(glGetUniformLocation(this->programId, uniformName));
        if (location != -1) {
            GL_CALL(glUseProgram(this->programId));
            GL_CALL(glUniformMatrix4fv(location, 1, false, glm::value_ptr(*mat)));
            GL_CALL(glUseProgram(0));
            return true;
        } else {
            fprintf(stderr, "%s: Static uniform named: %s was not found.\n", shaderTag, uniformName);
        }
    }
    return false;
}
GLenum ShaderCore::getResourceBlock(GLenum bufferType) {
    if (bufferType == GL_UNIFORM_BUFFER)
        return GL_UNIFORM_BLOCK;
    if (bufferType == GL_SHADER_STORAGE_BUFFER)
        return GL_SHADER_STORAGE_BLOCK;
    if (bufferType == GL_TRANSFORM_FEEDBACK_BUFFER)
        return GL_TRANSFORM_FEEDBACK_BUFFER;
    // if (bufferType == GL_ATOMIC_COUNTER_BUFFER)
    fprintf(stderr, "Buffer type was unexpected.\n");
    return GL_INVALID_ENUM;
}
bool ShaderCore::addBuffer(const char *bufferNameInShader, const GLenum bufferType, const GLuint bufferBindingPoint) {  // Each buffer must have a unique binding point
    // Purge any existing buffer which matches
    removeBuffer(bufferNameInShader);
    if (this->programId > 0) {
        // Get the uniform buffers index within the shader program
        GLenum blockType = getResourceBlock(bufferType);
        if (blockType == GL_INVALID_ENUM)
            return false;
        GLuint uniformBlockIndex = GL_CALL(glGetProgramResourceIndex(this->programId, blockType, bufferNameInShader));
        if (uniformBlockIndex != GL_INVALID_INDEX) {
            // Replace with new one
            // Can't use[] assignment constructor due to const elements
            BufferDetail bd = { bufferNameInShader, bufferType, bufferBindingPoint };
            // dynamicUniforms.erase(blockIndex);  // Why?
            auto rtn = buffers.emplace(uniformBlockIndex, bd);
            if (!rtn.second)fprintf(stderr, "%s: Buffer named: %s is already bound.\n", shaderTag, bufferNameInShader);
            GL_CALL(glUniformBlockBinding(this->programId, uniformBlockIndex, bufferBindingPoint));
            return true;
        } else if (strcmp(bufferNameInShader, Shaders::LIGHT_UNIFORM_BLOCK_NAME) && strcmp(bufferNameInShader, Shaders::MATERIAL_UNIFORM_BLOCK_NAME)) {  // Don't warn for known system buffers
            fprintf(stderr, "%s: Buffer named: %s was not found.\n", shaderTag, bufferNameInShader);
        }
    }
    lostBuffers.push_back({ bufferNameInShader, bufferType, bufferBindingPoint });
    return false;
}
bool ShaderCore::addTexture(const char *textureNameInShader, GLenum type, GLint textureName, GLuint textureUnit) {
    // Purge any existing buffer which matches
    for (auto a = textures.begin(); a != textures.end();) {
        if (a->second.name == static_cast<GLuint>(textureName)) {
            a = textures.erase(a);
        } else {
            // Check for collision of texture unit
            assert(a->first != static_cast<GLint>(textureUnit));
            ++a;
        }
    }
    UniformTextureDetail utd = { static_cast<GLuint>(textureName), type };
    textures.emplace(textureUnit, utd);
    GLint textureUnit_int = static_cast<GLint>(textureUnit);  // Samplers are set as int, not uint in shader
    return addStaticUniform(textureNameInShader, &textureUnit_int);
}
bool ShaderCore::removeDynamicUniform(const char *uniformName) {
    bool rtn = false;
    for (auto a = lostDynamicUniforms.begin(); a != lostDynamicUniforms.end();) {
        if (std::string((*a).uniformName) == std::string(uniformName)) {
            a = lostDynamicUniforms.erase(a);
            rtn = true;
        } else {
            ++a;
        }
    }
    for (auto a = dynamicUniforms.begin(); a != dynamicUniforms.end();) {
        if (std::string((*a).second.uniformName) == std::string(uniformName)) {
            a = dynamicUniforms.erase(a);
            rtn = true;
        } else {
            ++a;
        }
    }
    return rtn;
}
bool ShaderCore::removeStaticUniform(const char *uniformName) {
    bool rtn = false;
    for (auto a = staticUniforms.begin(); a != staticUniforms.end();) {
        if (std::string((*a).uniformName) == std::string(uniformName)) {
            a = staticUniforms.erase(a);
            rtn = true;
        } else {
            ++a;
        }
    }
    return rtn;
}
bool ShaderCore::removeTextureUniform(const char *uniformName) {
    bool rtn = false;
    for (auto a = staticUniforms.begin(); a != staticUniforms.end();) {
        if (std::string((*a).uniformName) == std::string(uniformName)) {
            textures.erase((*a).data.x);
            a = staticUniforms.erase(a);
            rtn = true;
        } else {
            ++a;
        }
    }
    return rtn;
}
bool ShaderCore::removeBuffer(const char *nameInShader) {
    bool rtn = false;
    for (auto a = lostBuffers.begin(); a != lostBuffers.end();) {
        if (std::string((*a).nameInShader) == std::string(nameInShader)) {
            a = lostBuffers.erase(a);
            rtn = true;
        } else {
            ++a;
        }
    }
    for (auto a = buffers.begin(); a != buffers.end();) {
        if (std::string((*a).second.nameInShader) == std::string(nameInShader)) {
            a = buffers.erase(a);
            rtn = true;
        } else {
            ++a;
        }
    }
    return rtn;
}
std::pair<int, GLenum> ShaderCore::findUniform(const char *uniformName, const int shaderProgram) {
    int uniformLocation = shaderProgram < 0 ? -1 : GL_CALL(glGetUniformLocation(shaderProgram, uniformName));
    // UniformIndex!=UniformLocation (Index required to get info about uniform, Location required to set val)
    GLuint uniformIndex = shaderProgram < 0 ? -1 : GL_CALL(glGetProgramResourceIndex(shaderProgram, GL_UNIFORM, uniformName));
    // GL_CALL(glGetUniformIndices(shaderProgram, 1, &uniformName, &uniformIndex));
    if (uniformLocation > -1 && uniformIndex != GL_INVALID_INDEX) {
        GLenum type;
        GLint size;  // Collect size, because its not documented that you can pass 0
        GL_CALL(glGetActiveUniform(shaderProgram, uniformIndex, 0, nullptr, &size, &type, nullptr));
        return std::pair<int, GLenum>(uniformLocation, type);
    }
    return  std::pair<int, GLenum>(-1, 0);
}
std::pair<int, GLenum> ShaderCore::findAttribute(const char *attributeName, const int shaderProgram) {
    int attribLocation = shaderProgram < 0 ? -1 : GL_CALL(glGetAttribLocation(shaderProgram, attributeName));
    // attribLocation != attribIndex (Index required to get info, Location required to set val)
    GLuint attribIndex = shaderProgram < 0 ? -1 : GL_CALL(glGetProgramResourceIndex(shaderProgram, GL_PROGRAM_INPUT, attributeName));
    if (attribLocation > -1 && attribIndex != GL_INVALID_INDEX) {
        GLenum type;
        GLint size;  // Collect size, because its not documented that you can pass 0
        GL_CALL(glGetActiveAttrib(shaderProgram, attribIndex, 0, nullptr, &size, &type, nullptr));
        return std::pair<int, GLenum>(attribLocation, type);
    }
    return  std::pair<int, GLenum>(-1, 0);
}
// Util
int ShaderCore::compileShader(const GLuint t_shaderProgram, GLenum type, std::vector<std::string> *shaderSourceFiles) {
    if (shaderSourceFiles->size() == 0) return false;
    //  Load shader files
    std::vector<char*> shaderSources;
    for (auto i : *shaderSourceFiles) {
        shaderSources.push_back(loadShaderSource(i.c_str()));
    }
    // Check for shaders that didn't load correctly
    for (auto i : shaderSources) {
        if (!i) {
            // Cleanup
            for (auto j : shaderSources) {
                free(j);
            }
            return -1;
        }
    }
    GLuint shaderId = createShader(type);
    GL_CALL(glShaderSource(shaderId, static_cast<GLsizei>(shaderSources.size()), &shaderSources[0], nullptr));
    GL_CALL(glCompileShader(shaderId));
    std::string shaderName = su::getFilenameFromPath(*(shaderSourceFiles->end() - 1));
    // Check for compile errors
    if (!this->checkShaderCompileError(shaderId, shaderName.c_str())) {
        // Cleanup
        for (auto j : shaderSources) {
            free(j);
        }
        return -1;
    }
    // Attach shader to program
    GL_CALL(glAttachShader(t_shaderProgram, shaderId));
    // Append to shaderTag
    if (shaderTag[0] == '\0') {
        shaderName = su::removeFileExt(shaderName);
    } else {
        shaderName = std::string(shaderTag) + std::string("-") + su::removeFileExt(shaderName);
        delete[] this->shaderTag;
    }
    this->shaderTag = new char[shaderName.length() + 1];
    snprintf(this->shaderTag, shaderName.length() + 1, "%s", shaderName.c_str());
    return static_cast<int>(findShaderVersion(*reinterpret_cast<std::vector<const char*>*>(&shaderSources)));
}
// If earlier than VS 2019
// #if defined(_MSC_VER) && _MSC_VER < 1920
// #include <filesystem>
// using std::tr2::sys::exists;
// using std::tr2::sys::path;
// #else
// // VS2019 requires this macro, as building pre c++17 cant use std::filesystem
// #define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
// #include <experimental/filesystem>
// using std::experimental::filesystem::v1::exists;
// using std::experimental::filesystem::v1::path;
// #endif
char* ShaderCore::loadShaderSource(const char* file) {
    // static std::string shadersRoot;
    // if (shadersRoot.empty()) {
    //     // Locate the root directory of the solution
    //     // Follow up tree a few layers checking for a shaders directory.
    //     shadersRoot = "./shaders/";
    //     for (unsigned int i = 0; i < 5; ++i) {
    //         if (exists(path(shadersRoot)))
    //             break;
    //         shadersRoot = std::string("./.") + shadersRoot;
    //     }
    //     if (!exists(path(shadersRoot)))
    //         shadersRoot = "./";
    // }
    //  If file path is 0 it is being omitted. kinda gross
    if (file != nullptr) {
        // std::string shaderPath = shadersRoot + file;
        // FILE* fptr = fopen(shaderPath.c_str(), "rb");  // Attempt with shader root
        // if (!fptr) {
        //     fptr = fopen(file, "rb");  // Attempt without shader root
        //     if (!fptr) {
        //         fprintf(stderr, "Shader source not found: %s\n", file);
        //         if (exitOnError) {
        //             getchar();
        //             exit(1);
        //         }
        //         return nullptr;
        //     }
        // }
        FILE* fptr = Resources::fopen(file, "rb");  // Attempt with shader root
        fseek(fptr, 0, SEEK_END);
        int64_t length = ftell(fptr);
        char* buf = static_cast<char*>(malloc(length + 1));  //  Allocate a buffer for the entire length of the file and a null terminator
        fseek(fptr, 0, SEEK_SET);
        fread(buf, length, 1, fptr);
        fclose(fptr);
        buf[length] = '\0';  //  Null terminator
        return buf;
    } else {
        return nullptr;
    }
}
bool ShaderCore::checkShaderCompileError(GLuint shaderId, const char* shaderPath) {
    GL_CHECK();
    GLint status;
    GL_CALL(glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status));
    if (status == GL_FALSE) {
        //  Get the length of the info log
        GLint len;
        GL_CALL(glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &len));
        //  Get the contents of the log message
        char* log = new char[len + 1];
        GL_CALL(glGetShaderInfoLog(shaderId, len, &len, log));
        //  Print the message
        fprintf(stderr, "Shader compilation error (%s) :\n", shaderPath);
        fprintf(stderr, "%s\n", log);
        delete[] log;
        if (exitOnError) {
            getchar();
            exit(1);
        }
        return false;
    }
    return true;
}
bool ShaderCore::checkProgramLinkError(const GLuint _programId) const {
    GL_CHECK();
    GLint status;
    GL_CALL(glGetProgramiv(_programId, GL_LINK_STATUS, &status));
    if (status == GL_FALSE) {
        //  Get the length of the info log
        GLint len = 0;
        GL_CALL(glGetProgramiv(_programId, GL_INFO_LOG_LENGTH, &len));
        //  Get the contents of the log message
        char* log = new char[len + 1];
        GL_CALL(glGetProgramInfoLog(_programId, len, nullptr, log));
        //  Print the message
        fprintf(stderr, "Program compilation error (%s):\n", shaderTag);
        fprintf(stderr, "%s\n", log);
        delete[] log;
        if (exitOnError) {
            getchar();
            exit(1);
        }
        return false;
    }
    return true;
}
unsigned int ShaderCore::findShaderVersion(std::vector<const char*> shaderSources) {
    static std::regex versionRegex("#version ([0-9]+)", std::regex::ECMAScript | std::regex_constants::icase);
    std::cmatch match;
    for (auto shaderSource : shaderSources)
        if (std::regex_search(shaderSource, match, versionRegex))
            return stoul(match[1]);
    return 0;
}
std::vector<std::string> *ShaderCore::buildFileVector(std::initializer_list<const char *> sources) {
    std::vector<std::string> *rtn = new std::vector<std::string>();

    for (auto i : sources) {
        rtn->push_back(std::string(i));
    }
    return rtn;
}
