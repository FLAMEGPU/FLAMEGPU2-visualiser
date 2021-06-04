#ifndef SRC_TEXTURE_TEXTURE2D_H_
#define SRC_TEXTURE_TEXTURE2D_H_

#include <memory>
#include <unordered_map>
#include <locale>
#include <string>
#include <algorithm>

#include <glm/vec2.hpp>

#include "util/GLcheck.h"
#include "interface/RenderTarget.h"
#include "texture/Texture.h"

namespace flamegpu {
namespace visualiser {


/**
 * Class representing two-dimensional textures
 * @note This class cannot be directly instantiated
 */
class Texture2D : public Texture, public RenderTarget {
 public:
    /**
     * Returns an immutable std::shared_ptr<ImageData> of the specified file
     * @param filepath The path to the image to be loaded
     * @param options A bitmask of options which correspond to various GL texture options
     * @param skipCache If false the returned Texture2D will be added to or loaded from the cache
     * @param folder A surplus folder to search for the texture in, useful for finding model textures
     */
    static std::shared_ptr<const Texture2D> load(const std::string &filepath, const std::string &folder, const uint64_t options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT, bool skipCache = false);
    static std::shared_ptr<const Texture2D> load(const std::string &filepath, const uint64_t options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT, bool skipCache = false);
    /**
     * Creates a mutable texture with the given paremeters
     * @param dimensions Size of the texture
     * @param format Format specification of the texture
     * @param data Data to fill the texture with
     * @param options Bitmask of filtering/wrapping opetions
     */
    static std::shared_ptr<Texture2D> make(const glm::uvec2 &dimensions, const Texture::Format &format, const void *data = nullptr, const uint64_t &options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT);
    /**
     * Creates a mutable texture with the given paremeters
     * @param dimensions Size of the texture
     * @param format Format specification of the texture
     * @param options Bitmask of filtering/wrapping opetions
     */
    static std::shared_ptr<Texture2D> make(const glm::uvec2 &dimensions, const Texture::Format &format, const uint64_t &options);
    /**
     * @param filePath The texture to check
     * @return True if the specified texture has currently been loaded and is cached
     */
    static bool isCached(const std::string &filePath);
    /**
     * Copy constructor, creates a mutable copy
     */
    Texture2D(const Texture2D& b);
    /**
     * Move constructor and assignment operators disabled, contains const elements
     */
    Texture2D(const Texture2D&& b) = delete;
    Texture2D& operator= (const Texture2D& b) = delete;
    Texture2D& operator= (const Texture2D&& b) = delete;
    /**
     * Resizes the texture
     * @param _dimensions New texture dimensions
     * @note This will lose existing data stored in the texture
     * @note This method is disabled for immutable textures
     */
    void resize(const glm::uvec2 &_dimensions) override { resize(_dimensions, nullptr); }
    using RenderTarget::resize;
    /**
     * Resizes the texture and updates the stored texture data
     * @param dimensions New texture dimensions
     * @param data Pointer to pixel data
     * @param size Optional validation method to ensure expected size of data is provided
     * @note This method is disabled for immutable textures
     */
    void resize(const glm::uvec2 &dimensions, void *data, size_t size = 0);
    /**
     * Updates the texture's data, without resizing the texture
     * @note This method is disabled for immutable textures
     */
    void setTexture(void *data, size_t size = 0);
    /**
     * Updates a subset of the texture's data, without resizing the texture
     * @param data Pointer to pixel data
     * @param dimensions New sub texture's dimensions
     * @param offset New sub texture's offset into the main texture
     * @param size Optional validation method to ensure expected size of data is provided
     * @note This method is disabled for immutable textures
     */
    void setSubTexture(void *data, glm::uvec2 dimensions, glm::ivec2 offset, size_t size = 0);
    /**
     * @return The dimensions of the currently alocated texture
     */
    glm::uvec2 getDimensions() const { return dimensions; }
    /**
     * @return The width of the currently alocated texture
     */
    unsigned int getWidth() const { return dimensions.x; }
    /**
     * @return The height of the currently alocated texture
     */
    unsigned int getHeight() const { return dimensions.y; }
    /**
     * @return boolean representing whether the texture is currently correct bound to it's allocated texture unit
     * @note This does not check whether it is the currently bound buffer!
     */
    bool isBound() const override;
    /**
     * Provides for RenderTarget's virtual getName(), simply defers call to Texture::getName();
     * @return The gl texture name as returned by glGenTextures()
     */
    GLenum getName() const override { return Texture::getName(); }

 protected:
	/**
	 * Private constructor
	 * @see load(...)
	 */
    Texture2D(std::shared_ptr<SDL_Surface> image, const std::string reference, const uint64_t &options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT);
	/**
	 * Private constructor
	 * @see make(...)
	 */
    Texture2D(const glm::uvec2 &dimensions, const Texture::Format &format, const void *data = nullptr, const uint64_t&options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT);

 private:
    /**
     * Used inside constructor to assign the instance a texture unit
     */
    static GLuint genTextureUnit();
    /**
      * Removed the named file from cache if present
     * @param filePath The texture to be purged
     */
    static void purgeCache(const std::string &filePath);
    /**
    * Returns the specified texture if present in the cache, else an empty shared_ptr is returned
    * @param filePath The texture to be returned
    */
    static std::shared_ptr<const Texture2D> loadFromCache(const std::string &filePath);
    /**
    * Cache for storing textures loaded from file
    */
    static std::unordered_map<std::string, std::weak_ptr<const Texture2D>> cache;
    /**
     * Dimensions of a face of the currently allocated texture
     */
    glm::uvec2 dimensions;
    /**
     * Immutable textures cannot be resized or their data changed
     * Texture's loaded from file will always be immutable
     * Mutable textures can be created from immutable by passing them via the copy constructor.
     */
    const bool immutable;
    static const char *RAW_TEXTURE_FLAG;
};

}  // namespace visualiser
}  // namespace flamegpu


#endif  // SRC_TEXTURE_TEXTURE2D_H_
