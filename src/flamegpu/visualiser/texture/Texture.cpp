#include "Texture.h"

#include <IL/il.h>
#include <SDL_surface.h>

#include <algorithm>
#include <cstdio>
#include <map>
#include <memory>
#include <string>

#include "flamegpu/visualiser/util/StringUtils.h"
#include "flamegpu/visualiser/util/Resources.h"

namespace flamegpu {
namespace visualiser {

namespace {
    /**
     * This saves us needing to link with ILU for error strings
     * Convert DevIL error enum to error string;
     */
    const char *ilErrorString(ILenum code) {
        switch (code) {
            case IL_NO_ERROR:
                return "No detectable error has occured.";
            case IL_INVALID_ENUM:
                return "An unacceptable enumerated value was passed to a function.";
            case IL_OUT_OF_MEMORY:
                return "Could not allocate enough memory in an operation.";
            case IL_FORMAT_NOT_SUPPORTED:
                return "The format a function tried to use was not able to be used by that function.";
            case IL_INTERNAL_ERROR:
                return "A serious error has occurred.Please e - mail DooMWiz with the conditions leading up to this error being reported.";
            case IL_INVALID_VALUE:
                return "An invalid value was passed to a function or was in a file.";
            case IL_ILLEGAL_OPERATION:
                return "The operation attempted is not allowable in the current state.The function returns with no ill side effects.";
            case IL_ILLEGAL_FILE_VALUE:
                return "An illegal value was found in a file trying to be loaded.";
            case IL_INVALID_FILE_HEADER:
                return "A file's header was incorrect.";
            case IL_INVALID_PARAM:
                return "An invalid parameter was passed to a function, such as a NULL pointer.";
            case IL_COULD_NOT_OPEN_FILE:
                return "Could not open the file specified.The file may already be open by another app or may not exist.";
            case IL_INVALID_EXTENSION:
                return "The extension of the specified filename was not correct for the type of image - loading function.";
            case IL_FILE_ALREADY_EXISTS:
                return "The filename specified already belongs to another file.To overwrite files by default, call ilEnable with the IL_FILE_OVERWRITE parameter.";
            case IL_OUT_FORMAT_SAME:
                return "Tried to convert an image from its format to the same format.";
            case IL_STACK_OVERFLOW:
                return "One of the internal stacks was already filled, and the user tried to add on to the full stack.";
            case IL_STACK_UNDERFLOW:
                return "One of the internal stacks was empty, and the user tried to empty the already empty stack.";
            case IL_INVALID_CONVERSION:
                return "An invalid conversion attempt was tried.";
            case IL_LIB_JPEG_ERROR:
                return "An error occurred in the libjpeg library.";
            case IL_LIB_PNG_ERROR:
                return "An error occurred in the libpng library.";
            case IL_UNKNOWN_ERROR:
            default:
                return "No function sets this yet, but it is possible(not probable) it may be used in the future.";
        }
    }
}  // namespace


std::map<SDL_Surface*, unsigned int> Texture::registered_surfaces;
bool Texture::IL_IS_INIT = false;

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
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_REPEAT;
    }
    if ((options & WRAP_CLAMP_TO_EDGE_U) == WRAP_CLAMP_TO_EDGE_U) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_CLAMP_TO_EDGE;
    }
    if ((options & WRAP_CLAMP_TO_BORDER_U) == WRAP_CLAMP_TO_BORDER_U) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_CLAMP_TO_BORDER;
    }
    if ((options & WRAP_MIRRORED_REPEAT_U) == WRAP_MIRRORED_REPEAT_U) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_MIRRORED_REPEAT;
    }
    if ((options & WRAP_MIRROR_CLAMP_TO_EDGE_U) == WRAP_MIRROR_CLAMP_TO_EDGE_U) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_MIRROR_CLAMP_TO_EDGE;
    }
    return rtn == GL_INVALID_ENUM ? GL_REPEAT : rtn;  // If none specified, return GL default
}
GLenum Texture::wrapOptionV() const {
    GLenum rtn = GL_INVALID_ENUM;
    if ((options & WRAP_REPEAT_V) == WRAP_REPEAT_V) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_REPEAT;
    }
    if ((options & WRAP_CLAMP_TO_EDGE_V) == WRAP_CLAMP_TO_EDGE_V) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_CLAMP_TO_EDGE;
    }
    if ((options & WRAP_CLAMP_TO_BORDER_V) == WRAP_CLAMP_TO_BORDER_V) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_CLAMP_TO_BORDER;
    }
    if ((options & WRAP_MIRRORED_REPEAT_V) == WRAP_MIRRORED_REPEAT_V) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
        rtn = GL_MIRRORED_REPEAT;
    }
    if ((options & WRAP_MIRROR_CLAMP_TO_EDGE_V) == WRAP_MIRROR_CLAMP_TO_EDGE_V) {
        visassert(rtn == GL_INVALID_ENUM);  // Invalid bitmask, multiple conflicting options passed
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
std::shared_ptr<SDL_Surface> Texture::findLoadImage(const std::string &imagePath) {
    // Attempt without appending extension
    std::shared_ptr<SDL_Surface> image = loadImage(imagePath, false, true);
    const char * IMAGE_EXTS[] = {"png", "jpg", "jpeg", "bmp", "tiff"};
    for (unsigned int i = 0; i < sizeof(IMAGE_EXTS) / sizeof(char*) && !image; i++) {
        image = loadImage(std::string(imagePath).append(".").append(IMAGE_EXTS[i]), false, true);
    }
    return image;
}
std::shared_ptr<SDL_Surface> Texture::loadImage(const std::string &imagePath, bool flipVertical, bool silenceErrors) {
    if (!IL_IS_INIT) {
        // Not documented whether it is safe to keep calling ilInit()
        // Is documented that calling ilShutdown() is unnecessary
        IL_IS_INIT = true;
        ilInit();
    }
    const std::string filePath = Resources::locateFile(imagePath);
    // Convert the DevIL image to SDL_Surface
    ILuint imageName;
    ilGenImages(1, &imageName);
    ilBindImage(imageName);
    if (!ilLoadImage(filePath.c_str())) {
        if (silenceErrors)
            return nullptr;
        THROW ResourceError("Texture::loadImage(): File '%s' could not be loaded!\n DevIL_Image error: %s", filePath.c_str(), ilErrorString(ilGetError()));
    }
    // Get image dimensions
    glm::uvec2 dims = glm::uvec2(ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT));
    int bytesPerPixel = ilGetInteger(IL_IMAGE_BPP);
    Uint32 rmask, gmask, bmask, amask = 0;
    int format = ilGetInteger(IL_IMAGE_FORMAT);
    if (format == IL_RGB) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
#else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
#endif
    } else if (format == IL_RGBA) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
#endif
    } else if (format == IL_BGR) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        bmask = 0xff000000;
        gmask = 0x00ff0000;
        rmask = 0x0000ff00;
#else
        bmask = 0x000000ff;
        gmask = 0x0000ff00;
        rmask = 0x00ff0000;
#endif
    } else if (format == IL_BGRA) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        bmask = 0xff000000;
        gmask = 0x00ff0000;
        rmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        bmask = 0x000000ff;
        gmask = 0x0000ff00;
        rmask = 0x00ff0000;
        amask = 0xff000000;
#endif
    } else {
        THROW ResourceError("Texture::loadImage(): File '%s' is of an unsupported format.", filePath.c_str());
    }
    ILubyte* il_data = ilGetData();
    // Convert to an SDL_surface
    std::shared_ptr<SDL_Surface> image = std::shared_ptr<SDL_Surface>(
        SDL_CreateRGBSurfaceFrom(il_data, dims.x, dims.y, bytesPerPixel * 8, dims.x * bytesPerPixel, rmask, gmask, bmask, amask)
        , Texture::freeSurface);
    registerSurface(image, imageName);
    ilBindImage(0);
    if (flipVertical)
        flipRows(image);
    return image;
}
unsigned int Texture::saveImage(void* data, unsigned int width, unsigned int height, const std::string& filepath) {
    if (!IL_IS_INIT) {
        // Not documented whether it is safe to keep calling ilInit()
        // Is documented that calling ilShutdown() is unnecessary
        IL_IS_INIT = true;
        ilInit();
    }
    ILuint imageName = 0;
    ilGenImages(1, &imageName);
    ilBindImage(imageName);
    ilTexImage(width, height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, data);
    if (ilGetError()) {
        fprintf(stderr, "Texture::saveImage(): DevIL returned an error: %s\n", ilErrorString(ilGetError()));
        return ilGetError();
    }
    ilEnable(IL_FILE_OVERWRITE);
    ilSaveImage(filepath.c_str());
    const unsigned int save_success = ilGetError();
    ilBindImage(0);
    ilDeleteImage(imageName);
    if (ilGetError()) {
        fprintf(stderr, "Texture::saveImage(): DevIL returned an error: %s\n", ilErrorString(ilGetError()));
        return ilGetError();
    }
    return save_success;
}
void Texture::registerSurface(const std::shared_ptr<SDL_Surface>& sdl_image, unsigned int devil_image) {
    registered_surfaces.emplace(sdl_image.get(), devil_image);
}
void Texture::freeSurface(SDL_Surface *sdl_image) {
    auto it = registered_surfaces.find(sdl_image);
    if (it != registered_surfaces.end()) {
        ilDeleteImage(it->second);
        registered_surfaces.erase(it);
    }
    SDL_FreeSurface(sdl_image);
}
bool Texture::flipRows(std::shared_ptr<SDL_Surface> img) {
    if (!img) {
        SDL_SetError("Surface is NULL");
        return false;
    }
    int pitch = img->pitch;
    int height = img->h;
    void* image_pixels = img->pixels;
    int index;
    void* temp_row;
    int height_div_2;

    temp_row = malloc(pitch);
    if (NULL == temp_row) {
        SDL_SetError("Not enough memory for image inversion");
        return false;
    }
    // if height is odd, don't need to swap middle row
    height_div_2 = static_cast<int>(height * .5);
    for (index = 0; index < height_div_2; index++) {
        // uses string.h
        memcpy(static_cast<uint8_t*>(temp_row),
            static_cast<uint8_t*>(image_pixels)+
            pitch * index,
            pitch);

        memcpy(
            static_cast<uint8_t*>(image_pixels)+
            pitch * index,
            static_cast<uint8_t*>(image_pixels)+
            pitch * (height - index - 1),
            pitch);
        memcpy(
            static_cast<uint8_t*>(image_pixels)+
            pitch * (height - index - 1),
            temp_row,
            pitch);
    }
    free(temp_row);
    return true;
}
void Texture::allocateTextureImmutable(std::shared_ptr<SDL_Surface> image, GLenum target) {
    target = target == 0 ? type : target;
    GL_CALL(glBindTexture(type, glName));
    // If the image is stored with a pitch different to width*bytes per pixel, temp change setting
    if (image->pitch / image->format->BytesPerPixel != image->w) {
        GL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, image->pitch / image->format->BytesPerPixel));
    }
    GL_CALL(glTexStorage2D(target, enableMipMapOption() ? 4 : 1, format.internalFormat, image->w, image->h));  // Must not be called twice on the same gl tex
    GL_CALL(glTexSubImage2D(target, 0, 0, 0, image->w, image->h, format.format, format.type, image->pixels));
    // Disable custom pitch
    if (image->pitch / image->format->BytesPerPixel != image->w) {
        GL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
    }
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

Texture::Format Texture::getFormat(std::shared_ptr<SDL_Surface> image) {
    visassert(image);
    switch (image->format->format) {
    case SDL_PIXELFORMAT_RGB332:
        return Format(GL_RGB, GL_R3_G3_B2, 1, GL_UNSIGNED_BYTE_3_3_2);
    case SDL_PIXELFORMAT_RGB444:
        return Format(GL_RGB, GL_RGB4, 2, GL_UNSIGNED_SHORT_4_4_4_4);
    case SDL_PIXELFORMAT_BGR555:
        return Format(GL_BGR, GL_RGB5, 2, GL_UNSIGNED_SHORT_5_5_5_1);
    case SDL_PIXELFORMAT_RGB555:
        return Format(GL_RGB, GL_RGB5, 2, GL_UNSIGNED_SHORT_5_5_5_1);
    case SDL_PIXELFORMAT_ABGR4444:
        return Format(GL_ABGR_EXT, GL_RGBA4, 2, GL_UNSIGNED_SHORT_4_4_4_4);
    case SDL_PIXELFORMAT_RGBA4444:
        return Format(GL_RGBA, GL_RGBA4, 2, GL_UNSIGNED_SHORT_4_4_4_4);
    case SDL_PIXELFORMAT_BGRA4444:
        return Format(GL_BGRA, GL_RGBA4, 2, GL_UNSIGNED_SHORT_4_4_4_4);
    case SDL_PIXELFORMAT_BGRA5551:
        return Format(GL_BGRA, GL_RGB5_A1, 2, GL_UNSIGNED_SHORT_5_5_5_1);
    case SDL_PIXELFORMAT_RGBA5551:
        return Format(GL_RGBA, GL_RGB5_A1, 2, GL_UNSIGNED_SHORT_5_5_5_1);
    case SDL_PIXELFORMAT_RGB565:
        return Format(GL_RGB, GL_RGB, 2, GL_UNSIGNED_SHORT_5_6_5);
    case SDL_PIXELFORMAT_BGR565:
        return Format(GL_BGR, GL_RGB, 2, GL_UNSIGNED_SHORT_5_6_5);
    case SDL_PIXELFORMAT_RGB24:
        return Format(GL_RGB, GL_RGB8, 3, GL_UNSIGNED_BYTE);  // Is this correct?
    case SDL_PIXELFORMAT_BGR24:
        return Format(GL_BGR, GL_RGB8, 3, GL_UNSIGNED_BYTE);  // Is this correct?
    case SDL_PIXELFORMAT_RGB888:
        return Format(GL_RGB, GL_RGB8, 3, GL_UNSIGNED_BYTE);  // Is this correct?
    case SDL_PIXELFORMAT_BGR888:
        return Format(GL_BGR, GL_RGB8, 3, GL_UNSIGNED_BYTE);  // Is this correct?
    case SDL_PIXELFORMAT_RGBA8888:
        return Format(GL_RGBA, GL_RGBA8, 4, GL_UNSIGNED_INT_8_8_8_8);
    case SDL_PIXELFORMAT_ABGR8888:
        return Format(GL_ABGR_EXT, GL_RGBA8, 4, GL_UNSIGNED_INT_8_8_8_8);
    case SDL_PIXELFORMAT_BGRA8888:
        return Format(GL_BGRA, GL_RGBA8, 4, GL_UNSIGNED_INT_8_8_8_8);
    // Possible if we bother to write reorder to RGBA functions
    case SDL_PIXELFORMAT_ARGB4444:
        // return Format(GL_RGBA, GL_RGBA4, GL_UNSIGNED_SHORT_4_4_4_4);
    case SDL_PIXELFORMAT_ARGB1555:
        // return Format(GL_RGBA, GL_RGB5_A1, GL_UNSIGNED_SHORT_5_5_5_1);
    case SDL_PIXELFORMAT_ABGR1555:
        // return Format(GL_RGBA, GL_RGB5_A1, GL_UNSIGNED_SHORT_5_5_5_1);
    case SDL_PIXELFORMAT_ARGB8888:
        // return Format(GL_RGBA, GL_RGBA8, GL_UNSIGNED_INT_8_8_8_8);
    // Unknown
    case SDL_PIXELFORMAT_ARGB2101010:
    case SDL_PIXELFORMAT_RGBX8888:
    case SDL_PIXELFORMAT_BGRX8888:
    case SDL_PIXELFORMAT_NV21:
    case SDL_PIXELFORMAT_NV12:
    case SDL_PIXELFORMAT_YVYU:
    case SDL_PIXELFORMAT_UYVY:
    case SDL_PIXELFORMAT_YUY2:
    case SDL_PIXELFORMAT_IYUV:
    case SDL_PIXELFORMAT_YV12:
    case SDL_PIXELFORMAT_UNKNOWN:
    case SDL_PIXELFORMAT_INDEX1LSB:
    case SDL_PIXELFORMAT_INDEX1MSB:
    case SDL_PIXELFORMAT_INDEX4LSB:
    case SDL_PIXELFORMAT_INDEX4MSB:
    case SDL_PIXELFORMAT_INDEX8:  // 8-bit palette?
    default:
        fprintf(stderr, "Unable to handle SDL_PIXELFORMAT: %d\n", image->format->format);
        assert(false);
    }
    return Format(0, 0, 0, 0);
}
}  // namespace visualiser
}  // namespace flamegpu

// Comment out this include if not making use of Shaders/ShaderCore
#include "flamegpu/visualiser/shader/ShaderCore.h"
#ifdef SRC_FLAMEGPU_VISUALISER_SHADER_SHADERCORE_H_
namespace flamegpu {
namespace visualiser {
bool ShaderCore::addTexture(const char *textureNameInShader, const std::shared_ptr<const Texture> &texture) {  // Treat it similar to texture binding points
    return addTexture(textureNameInShader, texture->getType(), texture->getName(), texture->getTextureUnit());
}
}  // namespace visualiser
}  // namespace flamegpu
#endif
