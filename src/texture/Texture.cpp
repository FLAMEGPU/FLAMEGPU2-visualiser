#include "texture/Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <algorithm>

#include "util/StringUtils.h"

Texture::ImageData::~ImageData() {
    if (data) {
        stbi_image_free(data);
        data = nullptr;
    }
}

// Ensure these remain lowercase without prepended '.'
const char* Texture::IMAGE_EXTS[] = {
    "tga",
    "png",
    "bmp",
    "jpg", "jpeg"
};

// Filter Ext Options
const uint64_t Texture::DISABLE_ANISTROPIC_FILTERING = 1ull << 0;
bool Texture::enableAnistropicOption() const {
    return  !((options & DISABLE_ANISTROPIC_FILTERING) == DISABLE_ANISTROPIC_FILTERING);
}

// Filter Min Options
const uint64_t Texture::FILTER_MIN_NEAREST                = 1ull << 1;
const uint64_t Texture::FILTER_MIN_LINEAR                    = 1ull << 2;
const uint64_t Texture::FILTER_MIN_NEAREST_MIPMAP_NEAREST = 1ull << 3;
const uint64_t Texture::FILTER_MIN_LINEAR_MIPMAP_NEAREST    = 1ull << 4;
const uint64_t Texture::FILTER_MIN_NEAREST_MIPMAP_LINEAR    = 1ull << 5;  // GL_Default value
const uint64_t Texture::FILTER_MIN_LINEAR_MIPMAP_LINEAR    = 1ull << 6;
GLenum Texture::filterMinOption() const {
    GLenum rtn = GL_INVALID_ENUM;
    if ((options & FILTER_MIN_NEAREST) == FILTER_MIN_NEAREST) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_NEAREST;
    }
    if ((options & FILTER_MIN_LINEAR) == FILTER_MIN_LINEAR) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_LINEAR;
    }
    if ((options & FILTER_MIN_NEAREST_MIPMAP_NEAREST) == FILTER_MIN_NEAREST_MIPMAP_NEAREST) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_NEAREST_MIPMAP_NEAREST;
    }
    if ((options & FILTER_MIN_LINEAR_MIPMAP_NEAREST) == FILTER_MIN_LINEAR_MIPMAP_NEAREST) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_LINEAR_MIPMAP_NEAREST;
    }
    if ((options & FILTER_MIN_NEAREST_MIPMAP_LINEAR) == FILTER_MIN_NEAREST_MIPMAP_LINEAR) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_NEAREST_MIPMAP_LINEAR;
    }
    if ((options & FILTER_MIN_LINEAR_MIPMAP_LINEAR) == FILTER_MIN_LINEAR_MIPMAP_LINEAR) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_LINEAR_MIPMAP_LINEAR;
    }
    return rtn == GL_INVALID_ENUM ? GL_NEAREST_MIPMAP_LINEAR : rtn;  // If none specified, return GL default
}

// Filter Mag Options
const uint64_t Texture::FILTER_MAG_LINEAR        = 1ull << 7;  // GL_Default value
const uint64_t Texture::FILTER_MAG_NEAREST    = 1ull << 8;
GLenum Texture::filterMagOption() const {
    GLenum rtn = GL_INVALID_ENUM;
    if ((options & FILTER_MAG_LINEAR) == FILTER_MAG_LINEAR) {
        assert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_LINEAR;
    }
    if ((options & FILTER_MAG_NEAREST) == FILTER_MAG_NEAREST) {
        assert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_NEAREST;
    }
    return rtn == GL_INVALID_ENUM ? GL_LINEAR : rtn;  // If none specified, return GL default
}

// Wrap Options
const uint64_t Texture::WRAP_REPEAT_U               = 1ull << 9;  // GL_Default value
const uint64_t Texture::WRAP_CLAMP_TO_EDGE_U        = 1ull << 10;
const uint64_t Texture::WRAP_CLAMP_TO_BORDER_U      = 1ull << 11;
const uint64_t Texture::WRAP_MIRRORED_REPEAT_U      = 1ull << 12;
const uint64_t Texture::WRAP_MIRROR_CLAMP_TO_EDGE_U = 1ull << 13;
const uint64_t Texture::WRAP_REPEAT_V               = 1ull << 14;  // GL_Default value
const uint64_t Texture::WRAP_CLAMP_TO_EDGE_V        = 1ull << 15;
const uint64_t Texture::WRAP_CLAMP_TO_BORDER_V      = 1ull << 16;
const uint64_t Texture::WRAP_MIRRORED_REPEAT_V      = 1ull << 17;
const uint64_t Texture::WRAP_MIRROR_CLAMP_TO_EDGE_V = 1ull << 18;
const uint64_t Texture::WRAP_REPEAT                 = WRAP_REPEAT_U | WRAP_REPEAT_V;
const uint64_t Texture::WRAP_CLAMP_TO_EDGE          = WRAP_CLAMP_TO_EDGE_U | WRAP_CLAMP_TO_EDGE_V;
const uint64_t Texture::WRAP_CLAMP_TO_BORDER        = WRAP_CLAMP_TO_BORDER_U | WRAP_CLAMP_TO_BORDER_V;
const uint64_t Texture::WRAP_MIRRORED_REPEAT        = WRAP_MIRRORED_REPEAT_U | WRAP_MIRRORED_REPEAT_V;
const uint64_t Texture::WRAP_MIRROR_CLAMP_TO_EDGE   = WRAP_MIRROR_CLAMP_TO_EDGE_U | WRAP_MIRROR_CLAMP_TO_EDGE_V;
GLenum Texture::wrapOptionU() const {
    GLenum rtn = GL_INVALID_ENUM;
    if ((options & WRAP_REPEAT_U) == WRAP_REPEAT_U) {
        assert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_REPEAT;
    }
    if ((options & WRAP_CLAMP_TO_EDGE_U) == WRAP_CLAMP_TO_EDGE_U) {
        assert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_CLAMP_TO_EDGE;
    }
    if ((options & WRAP_CLAMP_TO_BORDER_U) == WRAP_CLAMP_TO_BORDER_U) {
        assert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_CLAMP_TO_BORDER;
    }
    if ((options & WRAP_MIRRORED_REPEAT_U) == WRAP_MIRRORED_REPEAT_U) {
        assert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_MIRRORED_REPEAT;
    }
    if ((options & WRAP_MIRROR_CLAMP_TO_EDGE_U) == WRAP_MIRROR_CLAMP_TO_EDGE_U) {
        assert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_MIRROR_CLAMP_TO_EDGE;
    }
    return rtn == GL_INVALID_ENUM ? GL_REPEAT : rtn;  // If none specified, return GL default
}
GLenum Texture::wrapOptionV() const {
    GLenum rtn = GL_INVALID_ENUM;
    if ((options & WRAP_REPEAT_V) == WRAP_REPEAT_V) {
        assert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_REPEAT;
    }
    if ((options & WRAP_CLAMP_TO_EDGE_V) == WRAP_CLAMP_TO_EDGE_V) {
        assert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_CLAMP_TO_EDGE;
    }
    if ((options & WRAP_CLAMP_TO_BORDER_V) == WRAP_CLAMP_TO_BORDER_V) {
        assert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_CLAMP_TO_BORDER;
    }
    if ((options & WRAP_MIRRORED_REPEAT_V) == WRAP_MIRRORED_REPEAT_V) {
        assert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_MIRRORED_REPEAT;
    }
    if ((options & WRAP_MIRROR_CLAMP_TO_EDGE_V) == WRAP_MIRROR_CLAMP_TO_EDGE_V) {
        assert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_MIRROR_CLAMP_TO_EDGE;
    }
    return rtn == GL_INVALID_ENUM ? GL_REPEAT : rtn;  // If none specified, return GL default
}
// MipMap Options
const uint64_t Texture::DISABLE_MIPMAP = 1ull << 19;
bool Texture::enableMipMapOption() const {
    if (type == GL_TEXTURE_BUFFER || type == GL_TEXTURE_2D_MULTISAMPLE || type == GL_TEXTURE_RECTANGLE || type == GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
        return false;  // These formats don't support mimap
    return  !((options & DISABLE_MIPMAP) == DISABLE_MIPMAP);
}

void Texture::updateMipMap() {
    if (type == GL_TEXTURE_BUFFER || type == GL_TEXTURE_2D_MULTISAMPLE || type == GL_TEXTURE_RECTANGLE || type == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
        fprintf(stderr, "MipMap generation failed. Buffer, MultiSample and Array textures do not support mipmap!\n");
        return;
    }
#ifdef _DEBUG
    if (!enableMipMapOption()) {
        fprintf(stderr, "MipMap generation failed. MipMap option not enabled.\n");
        return;
    }
#endif
    GL_CALL(glBindTexture(type, glName));
    GL_CALL(glGenerateMipmap(type));
    GL_CALL(glBindTexture(type, 0));
}
// Constructors
Texture::Texture(GLenum type, GLuint textureUnit, const Format &format, const std::string &reference, uint64_t options, GLuint glName)
    : type(type)
    , glName(glName == 0?genTexName():glName)
    , textureUnit(textureUnit)
    , reference(reference)
    , format(format)
    , options(options)
    , externalTex(glName != 0) {
    visassert(textureUnit != 0);  // We reserve texture unit 0 for texture commands, because if we bind a texture to change settings we would knock the desired one out of the unit
    // Bind to texture unit (cant use bind() as includes debug call virtual fn)

    GL_CALL(glActiveTexture(GL_TEXTURE0 + this->textureUnit));
    GL_CALL(glBindTexture(this->type, this->glName));
    // Always return to Tex0 for doing normal texture work
    GL_CALL(glActiveTexture(GL_TEXTURE0));
}
Texture::~Texture() {
    if (!externalTex) {
        GL_CALL(glDeleteTextures(1, &glName));
    }
}

// General image loading utils
GLuint Texture::genTexName() {
    GLuint texName = 0;
    GL_CALL(glGenTextures(1, &texName));
    return texName;
}

void Texture::setOptions(uint64_t addOptions) {
    if ((addOptions&WRAP_REPEAT)||
        (addOptions&WRAP_CLAMP_TO_EDGE) ||
        (addOptions&WRAP_CLAMP_TO_BORDER) ||
        (addOptions&WRAP_MIRRORED_REPEAT) ||
        (addOptions&WRAP_MIRROR_CLAMP_TO_EDGE)) {
        // Unset current wrap setting
        options &= !(WRAP_REPEAT | WRAP_CLAMP_TO_EDGE | WRAP_CLAMP_TO_BORDER | WRAP_MIRRORED_REPEAT | WRAP_MIRROR_CLAMP_TO_EDGE);
    }
    if ((addOptions&FILTER_MAG_LINEAR) ||
        (addOptions&FILTER_MAG_NEAREST)) {
        // Unset current filter mag setting
        options &= !(FILTER_MAG_LINEAR | FILTER_MAG_NEAREST);
    }
    if ((addOptions&FILTER_MIN_NEAREST) ||
        (addOptions&FILTER_MIN_LINEAR) ||
        (addOptions&FILTER_MIN_NEAREST_MIPMAP_NEAREST) ||
        (addOptions&FILTER_MIN_LINEAR_MIPMAP_NEAREST) ||
        (addOptions&FILTER_MIN_NEAREST_MIPMAP_LINEAR) ||
        (addOptions&FILTER_MIN_LINEAR_MIPMAP_LINEAR)) {
        // Unset current filter min setting
        options &= !(FILTER_MIN_NEAREST | FILTER_MIN_LINEAR | FILTER_MIN_NEAREST_MIPMAP_NEAREST | FILTER_MIN_LINEAR_MIPMAP_NEAREST | FILTER_MIN_NEAREST_MIPMAP_LINEAR | FILTER_MIN_LINEAR_MIPMAP_LINEAR);
    }
    // Apply new settings
    options |= addOptions;
    applyOptions();
}
void Texture::unsetOptions(uint64_t removeOptions) {
    options &= !removeOptions;
    applyOptions();
}
void Texture::applyOptions() {
    GL_CALL(glBindTexture(type, glName));

    // Skip unsupported options for multisample
    if (type != GL_TEXTURE_2D_MULTISAMPLE) {
        if (enableAnistropicOption()) {  // Anistropic filtering (improves texture sampling at steep angle, especially visible with tiling patterns)
            GLfloat fLargest;
            GL_CALL(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest));
            GL_CALL(glTexParameterf(type, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest));
        }

        GL_CALL(glTexParameteri(type, GL_TEXTURE_MAG_FILTER, filterMagOption()));
        GL_CALL(glTexParameteri(type, GL_TEXTURE_MIN_FILTER, filterMinOption()));

        GL_CALL(glTexParameteri(type, GL_TEXTURE_WRAP_S, wrapOptionU()));
        GL_CALL(glTexParameteri(type, GL_TEXTURE_WRAP_T, wrapOptionV()));
        GL_CALL(glTexParameteri(type, GL_TEXTURE_WRAP_R, wrapOptionU()));  // Unused
    }

    if (enableMipMapOption()) {
        GL_CALL(glTexParameteri(type, GL_TEXTURE_MAX_LEVEL, 1000));  // Enable mip mapsdefault
        GL_CALL(glGenerateMipmap(type));
    } else {
        GL_CALL(glTexParameteri(type, GL_TEXTURE_MAX_LEVEL, 0));  // Disable mipmaps
    }
    GL_CALL(glBindTexture(type, 0));
}
std::shared_ptr<Texture::ImageData> Texture::findLoadImage(const std::string &imagePath) {
    // Attempt without appending extension
    std::shared_ptr<ImageData> image = loadImage(imagePath, false, true);
    for (int i = 0; i < sizeof(IMAGE_EXTS) / sizeof(char*) && !image; i++) {
        image = loadImage(std::string(imagePath).append(".").append(IMAGE_EXTS[i]), false, true);
    }
    return image;
}
std::shared_ptr<Texture::ImageData> Texture::loadImage(const std::string &imagePath, bool flipVertical, bool silenceErrors) {
    std::shared_ptr<ImageData> image = std::make_shared<ImageData>();
    stbi_set_flip_vertically_on_load(flipVertical);
    image->data = stbi_load(imagePath.c_str(), &image->width, &image->height, &image->channels, 0);
    if (!image) {
        if (!silenceErrors)
            fprintf(stderr, "Image file '%s' could not be read.\n", imagePath.c_str());
        return std::shared_ptr<ImageData>();
    }
    return image;
}
void Texture::allocateTextureImmutable(std::shared_ptr<ImageData> image, GLenum target) {
     target = target == 0 ? type : target;
     GL_CALL(glBindTexture(type, glName));
     // If the image is stored with a pitch different to width*bytes per pixel, temp change setting
     // (Disabled, stb_image never outputs files in this format)
     // if (image->pitch / image->format->BytesPerPixel != image->w)
     // {
     //     GL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, image->pitch / image->format->BytesPerPixel));
     // }
     GL_CALL(glTexStorage2D(target, enableMipMapOption() ? 4 : 1, format.internalFormat, image->width, image->height));  // Must not be called twice on the same gl tex
     GL_CALL(glTexSubImage2D(target, 0, 0, 0, image->width, image->height, format.format, format.type, image->data));
     // Disable custom pitch
     // if (image->pitch / image->format->BytesPerPixel != image->w)
     // {
     //     GL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
     // }
     GL_CALL(glBindTexture(type, 0));
}
void Texture::allocateTextureImmutable(const glm::uvec2 &dimensions, const void *data, GLenum target) {
    target = target == 0 ? type : target;
    GL_CALL(glBindTexture(type, glName));
    // Set custom algin, for safety
    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GL_CALL(glTexStorage2D(target, enableMipMapOption() ? 4 : 1, format.internalFormat, dimensions.x, dimensions.y));  // Must not be called twice on the same gl tex
    if (data) {
        GL_CALL(glTexSubImage2D(target, 0, 0, 0, dimensions.x, dimensions.y, format.format, format.type, data));
    }
    // Disable custom align
    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
    GL_CALL(glBindTexture(type, 0));
}
void Texture::allocateTextureMutable(const glm::uvec2 &dimensions, const void *data, GLenum target) {
    target = target == 0 ? type : target;
    GL_CALL(glBindTexture(type, glName));
    // Set custom align, for safety
    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GL_CALL(glTexImage2D(target, 0, format.internalFormat, dimensions.x, dimensions.y, 0, format.format, format.type, data));
    // Disable custom align
    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
    GL_CALL(glBindTexture(type, 0));
}
void Texture::setTexture(const void *data, const glm::uvec2 &dimensions, glm::ivec2 offset, GLenum target) {
    target = target == 0 ? type : target;
    GL_CALL(glBindTexture(type, glName));
    // Set custom align, for safety
    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    if (data) {
        GL_CALL(glTexSubImage2D(target, 0, offset.x, offset.y, dimensions.x, dimensions.y, format.format, format.type, data));
    }
    // Disable custom align
    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
    GL_CALL(glBindTexture(type, 0));
}

bool Texture::supportsExtension(const std::string &fileExtension) {
    const std::string _fileExtension = su::toLower(fileExtension);
    // Compare each extension with and without . prepended
    for (unsigned int i = 0; i < sizeof(IMAGE_EXTS) / sizeof(char*); ++i) {
        if (_fileExtension == IMAGE_EXTS[i])
            return true;
        if (_fileExtension == (std::string(".") + IMAGE_EXTS[i]))
            return true;
    }
    return false;
}
void Texture::bind() const {
#ifdef _DEBUG
    if (isBound())
        return;
#endif
    GL_CALL(glActiveTexture(GL_TEXTURE0 + textureUnit));
    GL_CALL(glBindTexture(type, glName));
    // Always return to Tex0 for doing normal texture work
    GL_CALL(glActiveTexture(GL_TEXTURE0));
}

Texture::Format Texture::getFormat(std::shared_ptr<ImageData> image) {
    visassert(image);
    switch (image->channels) {
    case 1:
        return Format(GL_RED, GL_R8, 1, GL_UNSIGNED_BYTE);  // Is this correct?
    case 2:
        return Format(GL_RG, GL_RG8, 2, GL_UNSIGNED_BYTE);  // Is this correct?
    case 3:
        return Format(GL_RGB, GL_RGB8, 3, GL_UNSIGNED_BYTE);  // Is this correct?
    case 4:
        return Format(GL_RGBA, GL_RGBA8, 4, GL_UNSIGNED_INT_8_8_8_8);
    default:
        fprintf(stderr, "Unable to handle channels: %d\n", image->channels);
        visassert(false);
    }
}
// Comment out this include if not making use of Shaders/ShaderCore
#include "shader/ShaderCore.h"
#ifdef SRC_SHADER_SHADERCORE_H_
bool ShaderCore::addTexture(const char *textureNameInShader, const std::shared_ptr<const Texture> &texture) {  // Treat it similar to texture binding points
    return addTexture(textureNameInShader, texture->getType(), texture->getName(), texture->getTextureUnit());
}
#endif
