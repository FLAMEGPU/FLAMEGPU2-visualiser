#include "texture/Texture2D.h"
#include <cassert>
// If earlier than VS 2019
#if defined(_MSC_VER) && _MSC_VER < 1920
#include <filesystem>
using std::tr2::sys::exists;
using std::tr2::sys::path;
#else
// VS2019 requires this macro, as building pre c++17 cant use std::filesystem
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
using std::experimental::filesystem::v1::exists;
using std::experimental::filesystem::v1::path;
#endif

#include <glm/gtx/component_wise.hpp>

#include "util/StringUtils.h"

const char *Texture2D::RAW_TEXTURE_FLAG = "Texture2D";
std::unordered_map<std::string, std::weak_ptr<const Texture2D>> Texture2D::cache;

/**
* Constructors
*/
// Texture2D::Texture2D(std::shared_ptr<SDL_Surface> image, const std::string reference, const uint64_t options)
//     : Texture(GL_TEXTURE_2D, genTextureUnit(), getFormat(image), reference, options)
//     , dimensions(image->w, image->h)
//     , immutable(true)
// {
//     visassert(image);
//     allocateTextureImmutable(image);
//     applyOptions();
// }
Texture2D::Texture2D(const glm::uvec2 &dimensions, const Texture::Format &format, const void *data, const uint64_t &options)
    : Texture(GL_TEXTURE_2D, genTextureUnit(), format, RAW_TEXTURE_FLAG, options)
    , dimensions(dimensions)
    , immutable(false) {
    allocateTextureMutable(dimensions, data);
    applyOptions();
}
/**
 * Copy/Assignment handling
 */
Texture2D::Texture2D(const Texture2D& b)
    : Texture(b.type, genTextureUnit(), b.format, std::string(b.reference).append("!"), b.options)
    , dimensions(b.dimensions)
    , immutable(false) {
    allocateTextureMutable(dimensions, nullptr);
    GL_CALL(glCopyImageSubData(
        b.glName, b.type, 0, 0, 0, 0,
        glName, type, 0, 0, 0, 0,
        b.dimensions.x, b.dimensions.y, 0));
    applyOptions();
}
/**
 * Factory
 */
std::shared_ptr<Texture2D> Texture2D::make(const glm::uvec2 &dimensions, const Texture::Format &format, const void *data, const uint64_t &options) {
    return std::shared_ptr<Texture2D>(new Texture2D(dimensions, format, data, options));
}
std::shared_ptr<Texture2D> Texture2D::make(const glm::uvec2 &dimensions, const Texture::Format &format, const uint64_t &options) {
    return make(dimensions, format, nullptr, options);
}
/**
 * Cache handling
 */
std::shared_ptr<const Texture2D> Texture2D::loadFromCache(const std::string &filePath) {
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
// std::shared_ptr<const Texture2D> Texture2D::load(const std::string &texPath, const std::string &modelFolder, const uint64_t options, bool skipCache)
// {
//     // Locate the file
//     std::string filePath;
//     // Raw exists
//     if (exists(path(texPath)))
//         filePath = texPath;
//     // Exists in model dir
//     else if (!modelFolder.empty()&&exists(path((modelFolder + texPath).c_str())))
//         filePath = modelFolder + texPath;
//     // Exists in model dir [path incorrect]
//     else if (!modelFolder.empty() && exists(path((modelFolder + su::getFilenameFromPath(texPath)).c_str())))
//         filePath = std::string(modelFolder) + su::getFilenameFromPath(texPath);
//
//     return load(filePath, options, skipCache);
// }
// std::shared_ptr<const Texture2D> Texture2D::load(const std::string &filePath, const uint64_t options, bool skipCache)
// {
//     // Attempt from cache
//     std::shared_ptr<const Texture2D> rtn;
//     if (!filePath.empty())
//     {
//         if (!skipCache)
//         {
//             rtn = loadFromCache(filePath);
//         }
//         // Load using loader
//         if (!rtn)
//         {
//             auto image = loadImage(filePath);
//             if (image)
//             {
//                 rtn = std::shared_ptr<const Texture2D>(new Texture2D(image, filePath, options),
//                     [&](Texture2D *ptr) {  // Custom deleter, which purges cache of item
//                     Texture2D::purgeCache(ptr->getReference());
//                     delete ptr;
//                 });
//             }
//         }
//         // If we've loaded something, store in cache
//         if (rtn&&!skipCache)
//         {
//             cache.emplace(filePath, rtn);
//         }
//     }
//     return rtn;
// }
bool Texture2D::isCached(const std::string &filePath) {
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
void Texture2D::purgeCache(const std::string &filePath) {
    auto a = cache.find(filePath);
    if (a != cache.end()) {
        // Erase record
        cache.erase(a);
    }
}
void Texture2D::resize(const glm::uvec2 &_dimensions, void *data, size_t size) {
    if (immutable) {
        THROW VisAssert("Texture2D::resize(): Textures loaded from images are immutable and cannot be changed.\n");
    }
    if (data&&size)
        visassert(size == format.pixelSize*compMul(_dimensions));
    this->dimensions = _dimensions;
    allocateTextureMutable(_dimensions, data);
    //  If image data has been updated, regen mipmap
    if (data) {
        GL_CALL(glBindTexture(type, glName));
        GL_CALL(glGenerateMipmap(type));
        GL_CALL(glBindTexture(type, 0));
    }
}
void Texture2D::setTexture(void *data, size_t size) {
    Texture2D::setSubTexture(data, this->dimensions, glm::ivec2(0), size);
}
void Texture2D::setSubTexture(void *data, glm::uvec2 _dimensions, glm::ivec2 offset, size_t size) {
    if (immutable) {
        THROW VisAssert("Texture2D::setSubTexture(): Textures loaded from images are immutable and cannot be changed.\n");
    }
    if (size)
        visassert(size == format.pixelSize*compMul(_dimensions));
    this->dimensions = _dimensions;
    Texture::setTexture(data, _dimensions, offset);
    //  If image data has been updated, regen mipmap
    if (data) {
        GL_CALL(glBindTexture(type, glName));
        GL_CALL(glGenerateMipmap(type));
        GL_CALL(glBindTexture(type, 0));
    }
}
/**
 * Required methods for handling texture units
 */
GLuint Texture2D::genTextureUnit() {
    static GLuint texUnit = 1;
    GLint maxUnits;
    GL_CALL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxUnits));  //  192 on Modern GPUs, spec minimum 80
#ifdef _DEBUG
    visassert(texUnit < static_cast<GLuint>(maxUnits));
#endif
    if (texUnit >= static_cast<GLuint>(maxUnits)) {
        texUnit = 1;
        fprintf(stderr, "Max texture units exceeded by GL_TEXTURE_2D, enable texture switching.\n");
        //  If we ever notice this being triggered, need to add a static flag to Shaders which tells it to rebind textures to units at use.
        //  Possibly even notifying it of duplicate units
    }
    return texUnit++;
}
bool Texture2D::isBound() const {
    GL_CALL(glActiveTexture(GL_TEXTURE0+textureUnit));
    GLint whichID;
    GL_CALL(glGetIntegerv(GL_TEXTURE_BINDING_2D, &whichID));
    return static_cast<GLuint>(whichID) == glName;
}
// Comment out this include if not making use of GaussianBlur
// #include "../shader/GaussianBlur.h"
// #ifdef __GaussianBlur_h__
// void GaussianBlur::blurR32F(std::shared_ptr<Texture2D> inTex, std::shared_ptr<Texture2D> outTex) {
// #ifdef _DEBUG
//     visassert(inTex->getDimensions() == outTex->getDimensions());
// #endif
//     blurR32F(inTex->getName(), outTex->getName(), inTex->getDimensions());
// }
// #endif
