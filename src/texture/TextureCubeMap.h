#ifndef SRC_TEXTURE_TEXTURECUBEMAP_H_
#define SRC_TEXTURE_TEXTURECUBEMAP_H_
#include <unordered_map>
#include <string>
#include <memory>

#include <glm/vec2.hpp>

#include "Texture.h"

#define CUBE_MAP_FACE_COUNT 6
class TextureCubeMap : public Texture {
 public:
    static const char *SKYBOX_PATH;
    struct CubeMapParts {
        CubeMapParts(GLenum target, const char *name)
            : target(target), name(name) {}
        GLenum target;
        const char *name;
    };
    static const CubeMapParts FACES[CUBE_MAP_FACE_COUNT];

    /**
     * Returns a std::shared_ptr<TextureCubeMap> from the specified directory
     * @param filepath The path to the image to be loaded
     * @param options A bitmask of options which correspond to various GL texture options
     * @param skipCache If false the returned Texture2D will be added to or loaded from the cache
     */
    // static std::shared_ptr<const TextureCubeMap> load(const std::string &filepath, const uint64_t options = FILTER_MIN_LINEAR_MIPMAP_LINEAR | FILTER_MAG_LINEAR | WRAP_REPEAT, bool skipCache = false);
    /**
     * @param filePath The cubemap texture to check
     * @return True if the specified cubemap texture has currently been loaded and is cached
     */
    static bool isCached(const std::string &filePath);
    /**
     * Copy constructor, creates a mutable copy
     */
    TextureCubeMap(const TextureCubeMap& b);
    /**
     * Move constructor and assignment operators disabled, contains const elements
     */
    TextureCubeMap(const TextureCubeMap&& b) = delete;
    TextureCubeMap& operator= (const TextureCubeMap& b) = delete;
    TextureCubeMap& operator= (const TextureCubeMap&& b) = delete;
    /**
     * @return boolean representing whether the texture is currently correct bound to it's allocated texture unit
     * @note This does not check whether it is the currently bound buffer!
     */
    bool isBound() const override;

 private:
    /**
     * Private constructor
     * @see load(const std::string &, const uint64_t, bool)
     */
    // TextureCubeMap(std::shared_ptr<SDL_Surface> image[CUBE_MAP_FACE_COUNT], const std::string reference, const uint64_t options);
    /**
     * Used inside constructor to assign the instance a texture unit
     */
    static GLuint genTextureUnit();
    /**
     * Removed the named file from cache if present
     * @param filePath The cubemap texture to be purged
     */
    static void purgeCache(const std::string &filePath);
    /**
     * Returns the specified cubemap texture if present in the cache, else an empty shared_ptr is returned
     * @param filePath The cubemap texture to be returned
     */
    static std::shared_ptr<const TextureCubeMap> loadFromCache(const std::string &filePath);
    /**
     * Cache for storing textures loaded from file
     */
    static std::unordered_map<std::string, std::weak_ptr<const TextureCubeMap>> cache;
    /**
     * Dimensions of a face of the cube map
     * This value must be shared by all faces
     */
    const glm::uvec2 faceDimensions;
    /**
     * Immutable textures cannot be resized or their data changed
     * Texture's loaded from file will always be immutable
     * Mutable textures can be created from immutable by passing them via the copy constructor.
     */
    const bool immutable;
};
#endif  // SRC_TEXTURE_TEXTURECUBEMAP_H_
