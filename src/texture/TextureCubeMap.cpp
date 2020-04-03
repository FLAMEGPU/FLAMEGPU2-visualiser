#include "texture/TextureCubeMap.h"
#include <glm/gtx/component_wise.hpp>

/**
 * Static members
 */
// The stock skybox path (used as the default cube map texture
const char *TextureCubeMap::SKYBOX_PATH = "../textures/skybox/";
// The file name (without the file type) to face mapping used by cube maps
const TextureCubeMap::CubeMapParts TextureCubeMap::FACES[] = {
    CubeMapParts(GL_TEXTURE_CUBE_MAP_POSITIVE_X, "left"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "right"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "up"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "down"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "front"),
    CubeMapParts(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "back")
};
std::unordered_map<std::string, std::weak_ptr<const TextureCubeMap>> TextureCubeMap::cache;
/**
 * Constructors
 */
// TextureCubeMap::TextureCubeMap(std::shared_ptr<SDL_Surface> images[CUBE_MAP_FACE_COUNT], const std::string reference, const uint64_t options)
//     :Texture(GL_TEXTURE_CUBE_MAP, genTextureUnit(), getFormat(images[0]), reference, options)
//     , faceDimensions(images[0]->w, images[0]->h)
//     , immutable(true)
// {
//     GL_CALL(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));
//     allocateTextureImmutable(faceDimensions);
//     for (unsigned int i = 0; i < sizeof(FACES) / sizeof(CubeMapParts); i++)
//     {
//         visassert(images[i]);
//         visassert(images[i]->w == faceDimensions.x);  // All must share dimensions and pixel format
//         visassert(images[i]->h == faceDimensions.y);
//         visassert(getFormat(images[i]) == format);
//         setTexture(images[i]->pixels, faceDimensions, glm::ivec2(0), FACES[i].target);
//     }
//     applyOptions();
// }
/**
 * Copy/Assignment handling
 */
TextureCubeMap::TextureCubeMap(const TextureCubeMap& b)
    : Texture(b.type, genTextureUnit(), b.format, std::string(b.reference).append("!"), b.options)
    , faceDimensions(b.faceDimensions)
    , immutable(false) {
    // Allocate each face
    for (unsigned int i = 0; i < sizeof(FACES) / sizeof(CubeMapParts); i++) {
        allocateTextureMutable(faceDimensions, nullptr, FACES[i].target);
    }
    // Copy all faces simultaneously
    GL_CALL(glCopyImageSubData(
        b.glName, b.type, 0, 0, 0, 0,
        glName, type, 0, 0, 0, 0,
        b.faceDimensions.x, b.faceDimensions.y, 6));
    applyOptions();
}
/**
* Cache handling
*/
std::shared_ptr<const TextureCubeMap> TextureCubeMap::loadFromCache(const std::string &filePath) {
    auto a = cache.find(filePath);
    if (a != cache.end()) {
        if (auto b = a->second.lock()) {
            return b;
        }
        // Weak pointer has expired, erase record
        // This should be redundant, if custom deleter has been used
        cache.erase(a);
    }
    return nullptr;
}
// std::shared_ptr<const TextureCubeMap> TextureCubeMap::load(const std::string &filePath, const uint64_t options, bool skipCache)
// {
//     // Attempt from cache
//     std::shared_ptr<const TextureCubeMap> rtn;
//     if (!skipCache)
//     {
//         rtn = loadFromCache(filePath);
//     }
//     // Load using loader
//     if (!rtn)
//     {
//         // Ensure file path contains trailing slash
//         std::string _filePath = (filePath.back() == '\\' || filePath.back() == '/') ? filePath : std::string(filePath).append("// ");
//         // Build array of images, checking each has loaded correctly
//         std::shared_ptr<SDL_Surface> *images = new std::shared_ptr<SDL_Surface>[CUBE_MAP_FACE_COUNT];
//         for (unsigned int i = 0; i < sizeof(FACES) / sizeof(CubeMapParts); i++)
//         {
//             images[i] = findLoadImage(std::string(_filePath).append(FACES[i].name));
//             if (!images[i])
//                 return rtn;
//         }
//         // Pass to constructor
//         rtn = std::shared_ptr<const TextureCubeMap>(new TextureCubeMap(images, filePath, options),
//             [&](TextureCubeMap *ptr) {  // Custom deleter, which purges cache of item
//             TextureCubeMap::purgeCache(ptr->getReference());
//             delete ptr;
//         });
//         // Delete images
//         delete[] images;
//         images = nullptr;
//     }
//     // If we've loaded something, store in cache
//     if (rtn&&!skipCache)
//     {
//         cache.emplace(filePath, rtn);
//     }
//     return rtn;
// }
bool TextureCubeMap::isCached(const std::string &filePath) {
    auto a = cache.find(filePath);
    if (a != cache.end()) {
        if (a->second.lock())
            return true;
        // Weak pointer has expired, erase record
        // This should be redundant, if custom deleter has been used
        cache.erase(a);
    }
    return false;
}
void TextureCubeMap::purgeCache(const std::string &filePath) {
    auto a = cache.find(filePath);
    if (a != cache.end()) {
        // Erase record
        cache.erase(a);
    }
}
/**
 * Required methods for handling texture units
 */
GLuint TextureCubeMap::genTextureUnit() {
    static GLuint texUnit = 1;
    GLint maxUnits;
    GL_CALL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits));  //  192 on Modern GPUs, spec minimum 80
#ifdef _DEBUG
    visassert(texUnit < static_cast<GLuint>(maxUnits));
#endif
    if (texUnit >= static_cast<GLuint>(maxUnits)) {
        texUnit = 1;
        fprintf(stderr, "Max texture units exceeded by GL_TEXTURE_CUBE_MAP, enable texture switching.\n");
        //  If we ever notice this being triggered, need to add a static flag to Shaders which tells it to rebind textures to units at use.
        //  Possibly even notifying it of duplicate units
    }
    return texUnit++;
}
bool TextureCubeMap::isBound() const {
    GL_CALL(glActiveTexture(GL_TEXTURE0 + textureUnit));
    GLint whichID;
    GL_CALL(glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &whichID));
    return static_cast<GLuint>(whichID) == glName;
}
