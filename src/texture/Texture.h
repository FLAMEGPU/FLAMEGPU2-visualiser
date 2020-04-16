#ifndef SRC_TEXTURE_TEXTURE_H_
#define SRC_TEXTURE_TEXTURE_H_

#include <string>
#include <memory>

#include <glm/vec2.hpp>

#include "../util/GLcheck.h"

/**
 * Shell texture class providing various utility methods for subclasses
 * SDL_EXP intends for each texture type to assign each instance a unique texture unit
 * At this time it's not expected that any use cases will require greater than GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
 * @note This class cannot be directly instantiated
 */
class Texture {
 protected:
    struct ImageData {
        int width;
        int height;
        int channels;
        unsigned char *data = nullptr;
        ~ImageData();
    };

 public:
    /**
     * This structure holds the necessary Enums for calling various OpenGL texture funtions
     * format, internal format and type, see docs of glTexImage2D() or similar for explanation
     * pixelSize is the size of a single pixel in bytes, this is included for convenience (and accuracy)
     */
    struct Format {
        Format(GLenum format, GLenum internalFormat, size_t pixelSize, GLenum type)
            : format(format), internalFormat(internalFormat), type(type), pixelSize(pixelSize) { }
        bool operator == (const Format& other) const {
            return
                other.format == this->format&&
                other.internalFormat == this->internalFormat&&
                other.type == this->type;
        }
        const GLenum format;
        const GLenum internalFormat;
        const GLenum type;
        const size_t pixelSize;  // inBytes
    };
    /**
     * Options
     */
    // Toggles anistropic filtering, this is good at steep angles
    static const uint64_t DISABLE_ANISTROPIC_FILTERING;
    // Selects the minifying filter, used when the texture is being downscaled
    static const uint64_t FILTER_MIN_NEAREST;
    static const uint64_t FILTER_MIN_LINEAR;
    static const uint64_t FILTER_MIN_NEAREST_MIPMAP_NEAREST;
    static const uint64_t FILTER_MIN_LINEAR_MIPMAP_NEAREST;
    static const uint64_t FILTER_MIN_NEAREST_MIPMAP_LINEAR;
    static const uint64_t FILTER_MIN_LINEAR_MIPMAP_LINEAR;
    // Selects the magnifying filter, used when the texture is being upscaled
    static const uint64_t FILTER_MAG_LINEAR;
    static const uint64_t FILTER_MAG_NEAREST;
    // Selects how texture coordinates are wrapped
    static const uint64_t WRAP_REPEAT;
    static const uint64_t WRAP_CLAMP_TO_EDGE;
    static const uint64_t WRAP_CLAMP_TO_BORDER;
    static const uint64_t WRAP_MIRRORED_REPEAT;
    static const uint64_t WRAP_MIRROR_CLAMP_TO_EDGE;
    static const uint64_t WRAP_REPEAT_U;
    static const uint64_t WRAP_CLAMP_TO_EDGE_U;
    static const uint64_t WRAP_CLAMP_TO_BORDER_U;
    static const uint64_t WRAP_MIRRORED_REPEAT_U;
    static const uint64_t WRAP_MIRROR_CLAMP_TO_EDGE_U;
    static const uint64_t WRAP_REPEAT_V;
    static const uint64_t WRAP_CLAMP_TO_EDGE_V;
    static const uint64_t WRAP_CLAMP_TO_BORDER_V;
    static const uint64_t WRAP_MIRRORED_REPEAT_V;
    static const uint64_t WRAP_MIRROR_CLAMP_TO_EDGE_V;
    // Toggle's the use of mip maps (4 levels are used with mipmaps, this is not currently changeable)
    static const uint64_t DISABLE_MIPMAP;
    /**
     * @return The GLenum representing the type of texture, e.g. GL_TEXTURE_2D, GL_TEXTURE_CUBEMAP, GL_TEXTURE_BUFFER
     */
    GLenum getType() const { return type; }
    /**
     * @return The unique name given to the texture by OpenGL (allocated by glGenTextures())
     */
    virtual GLenum getName() const { return glName; }
    /**
     * @return The texture unit we have assigned to this texture
     * @note Texture's of different type's may share this texture unit, using the appropriate sampler in shaders will select the correct texture
     * @note Currently we are not set up to have more textures of any type than the number of unique units (~92)
     */
    GLenum getTextureUnit() const { return textureUnit; }
    /**
     * @return The name assigned to the texture
     * @note For textures loaded from file, this will be the file path
     * @note For raw textures this will simply be the string of the enum returned by getType()
     */
    std::string getReference() const { return reference; }
    /**
     * @return Whether the texture is the currently bound texture of it's given type to the assigned texture unit
     */
    virtual bool isBound() const = 0;
    /**
     * Binds the texture to it's currently assigned texture unit
     */
    void bind() const;
    /**
     * Returns whether the anistropic filtering option is enabled according to the selected options
     */
    bool enableAnistropicOption() const;
    /**
     * Returns the GLenum of the selected minifying filter option
     */
    GLenum filterMinOption() const;
    /**
     * Returns the GLenum of the selected magnifying filter option
     */
    GLenum filterMagOption() const;
    /**
     * Returns the GLenum of the selected texture wrap option
     */
    GLenum wrapOptionU() const;
    GLenum wrapOptionV() const;
    /**
     * Returns whether mipmapping is enabled according to the selected options
     */
    bool enableMipMapOption() const;
    /**
     * Enables any passed options
     * @param addOptions The options to enable
     */
    void setOptions(uint64_t addOptions);
    /**
     * Disables any passed options
     * @param removeOptions The options to disable
     */
    void unsetOptions(uint64_t removeOptions);
    /**
     * Regenerate's the texture's mip map
     * @note This does nothing for texture's with mipmap disabled
     * @note Texture buffer and array texture's do not support mipmapping
     */
    void updateMipMap();
    /**
     * @param fileExtension A given string of a file extension
     * @return Wether the image loader is capable of handling the given extension
     * @note Indifferent to case-sensitivity and the inclusion of initial '.'
     */
    static bool supportsExtension(const std::string &fileExtension);
    /**
     * Virtual destructor, deallocate the OpenGL texture
     * @note Does not deallocate external texture's
     */
    virtual ~Texture();

 protected:
    /**
     * Remove all copy/move/assignment contructors
     * Subclasses should call the regular constructor from any copy constructors.
     */
    Texture(const Texture& b) = delete;
    Texture(const Texture&& b) = delete;
    Texture& operator= (const Texture& b) = delete;
    Texture& operator= (const Texture&& b) = delete;
    /**
     * @param type The texture type e.g. GL_TEXTURE_2D
     * @param textureUnit The chosen texture unit to be used by the texture
     * @param format The internal format specifiers of the texture
     * @param reference A string to identifyy the texture
     * @param options The filtering/wrapping/mipmap options for how the texture should be handled
     * @param glName If non 0 this will force the texture to use an external GL texture, rather than allocate it's own
     */
    Texture(GLenum type, GLuint textureUnit, const Format &format, const std::string &reference, uint64_t options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT, GLuint glName = 0);
    void applyOptions();
    /**
     * Copies a texture from an SDL_Surface into the host object's GL texture
     * @param image The SDL_Surface to upload to the GL texture
     * @param target The texture target, if left as default 'type' will be used, only fancy textures like cube map require this parameter
     * @note The allocated texture is immutable, so cannot be resized
     */
    void allocateTextureImmutable(std::shared_ptr<ImageData> image, GLenum target = 0);
    /**
     * Allocates an immutable texture, this cannot be later resized. Optionally fills with provided data
     * @param dimensions The dimensions of the image to be allocated
     * @param data The (optional) data to fill the allocated image with
     * @param target The texture target, if left as default 'type' will be used, only fancy textures like cube map require this parameter
     * @note The allocated texture is immutable, so cannot be resized
     */
    void allocateTextureImmutable(const glm::uvec2 &dimensions, const void *data = nullptr, GLenum target = 0);
    /**
     * Allocates a mutable texture, this can be later resized by calling the function again. Optionally fills with provided data
     * @param dimensions The dimensions of the image to be allocated
     * @param data The (optional) data to fill the allocated image with
     * @param target The texture target, if left as default 'type' will be used, only fancy textures like cube map require this parameter
     */
    void allocateTextureMutable(const glm::uvec2 &dimensions, const void *data = nullptr, GLenum target = 0);
    /**
     * Copies the specified data into the texture
     * @param data Pointer to the texture data on host
     * @param dimensions The dimensions of the image to be copied
     * @param offset The optional offset into the texture where to write the image (used if copying a sub image)
     * @param target The texture target, if left as default 'type' will be used, only fancy textures like cube map require this parameter
     */
    void setTexture(const void *data, const glm::uvec2 &dimensions, glm::ivec2 offset = glm::ivec2(0), GLenum target = 0);
    /**
     * The GLenum representation of the texture type. e.g. GL_TEXTURE_2D, GL_TEXTURE_BUFFER etc
     */
    const GLenum type;
    /**
     * The OpenGL name of the texture as allocated by glGenTextures()
     */
    const GLuint glName;
    /**
     * The (currently) unique texture unit allocated to this texture instance
     * Total number of texture units available per type is found via GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
     * Each type of texture has it's own set of texture units, so they may share units
     * Possible in future to allow duplicated units by setting a flag within ShaderCore that identifies duplicated units
     */
    const GLuint textureUnit;
    /**
     * String representation of the texture, only currently used by textures loaded from disk
     */
    const std::string reference;
    /**
     * This structure holds the necessary Enums for calling various OpenGL texture funtions
     * format, internal format and type, see docs of glTexImage2D() or similar for explanation
     * pixelSize is the size of a single pixel in bytes, this is included for convenience (and accuracy)
     */
    const Format format;
    /**
     * Bitmask of options to be applied to the texture
     * Primarily filtering, magnifying and wrapping settings
     * See the various constants in the rest of this class definition
     */
    uint64_t options;
    /**
     * Returns the first image found at the provided path
     * This method attempts all the suffices stored in Texture::IMAGE_EXTS
     * @param imagePath Path to search for images
     */
    static std::shared_ptr<ImageData> findLoadImage(const std::string &imagePath);
    /**
     * Returns the specified image
     * @param imagePath Path to search for images
     * @param flipVertical Vertically flips the image. This is useful because most images are indexed from the top, whereas GL indexes from the bottom
     * @param silenceErrors If true, will not print errors to console (used by findLoadImage() to reduce error spam)
     */
    static std::shared_ptr<ImageData> loadImage(const std::string &imagePath, bool flipVertical = true, bool silenceErrors = false);
    /**
     * Attempts to identify the format of the provided SDL_Surface
     * @param image SDL_Surface to identify format
     * @return A Format struct containing image format data
     */
    static Format getFormat(std::shared_ptr<ImageData> image);
    /**
     * Array of image extensions supported by SDL_SURFACE
     */
    static const char *IMAGE_EXTS[5];

 private:
    /**
     * Calls glGenTextures() returning the generated texture name
     */
    static GLuint genTexName();
    /**
     * If true this value tells the destructor not to delete the texture at object destruction
     */
    const bool externalTex;
};

#endif  // SRC_TEXTURE_TEXTURE_H_
