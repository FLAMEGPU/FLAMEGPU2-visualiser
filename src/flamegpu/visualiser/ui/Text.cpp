#include "Text.h"

#include <freetype/ftglyph.h>

#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

#include "flamegpu/visualiser/util/warnings.h"
#include "flamegpu/visualiser/util/fonts.h"

DISABLE_WARNING_PUSH
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNING_POP

#include "flamegpu/visualiser/shader/Shaders.h"

namespace flamegpu {
namespace visualiser {

Text::Text(const char *string, unsigned int fontHeight, glm::vec3 color, char const *fontFile, unsigned int faceIndex)
    :Text(string, fontHeight, glm::vec4(color, 1.0f), fontFile, faceIndex) {}
Text::Text(const char *_string, unsigned int fontHeight, glm::vec4 color, char const *fontFile, unsigned int faceIndex)
    : Overlay(std::make_shared<Shaders>(Stock::Shaders::TEXT))
    , printMono(false)
    , padding(5)
    , lineSpacing(-0.1f)
    , color(color)
    , backgroundColor(0.0f)
    , library()
    , font()
    , string(0)
    , fontHeight(fontHeight)
    , wrapDistance(800)
    , tex(std::make_shared<TextureString>()) {
    getShaders()->addStaticUniform("_col", glm::value_ptr(this->color), 4);
    getShaders()->addStaticUniform("_backCol", glm::value_ptr(this->backgroundColor), 4);
    getShaders()->addTexture("_texture", tex);
    if (!fontFile) {
        fontFile = fonts::findFont({"Arial"}, fonts::GenericFontFamily::SANS).c_str();
    }
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        THROW FreeTypeError("An unexpected error occurred whilst initialising FreeType: %i\n", error);
    }
    error = FT_New_Face(library,
        fontFile,
        faceIndex,
        &font);
    if (error == FT_Err_Unknown_File_Format) {
        // fprintf(stderr, "The font file %s is of an unsupported format, defaulting to Arial\n", fontFile);
        fontFile = fonts::findFont({"Arial"}, fonts::GenericFontFamily::SANS).c_str();
        error = FT_New_Face(library,
            fontFile,
            0,
            &this->font);
    }
    if (error) {
        // fprintf(stderr, "An unexpected error occurred whilst loading font file %s: %i, defaulting to Arial\n", fontFile, error);
        fontFile = fonts::findFont({"Arial"}, fonts::GenericFontFamily::SANS).c_str();
        error = FT_New_Face(library,
            fontFile,
            0,
            &this->font);
    }
    error = FT_Set_Pixel_Sizes(
        this->font,   /* handle to face object */
        0,      /* pixel_width           */
        this->fontHeight);/* pixel_height          */
    if (error) {
        THROW FreeTypeError("An unexpected error occurred whilst setting font size : % i\n", error);
    }
    setString(_string);
}
Text::~Text() {
    if (this->font)
        FT_Done_Face(this->font);
    if (this->library)
        FT_Done_FreeType(this->library);
    if (this->string)
        free(this->string);
}
void Text::reload() {
    recomputeTex();
}
/**
 * Structure used within Text::recomputeTex() to keep info about each glyph in a single structure
 */
struct  TGlyph {
    FT_UInt    index;  /* glyph index                  */
    FT_Vector  pos;    /* glyph origin on the baseline */
    FT_Glyph   image;  /* glyph image                  */
    char       c;      /* char                         */
    int        line;   /* Line number                  */
};
void Text::recomputeTex() {
    setStringLen();
    if (stringLen <= 0) return;
    FT_Error error;

    // First load and position all glyphs on a straight line
    TGlyph *glyphs = reinterpret_cast<TGlyph *>(malloc(stringLen * sizeof(TGlyph)));  // glyph image
    bool use_kerning = FT_HAS_KERNING(font) != 0;
    unsigned int previous = 0;

    int penX = 0, penY = 0;
    unsigned int i = 0;
    for (unsigned int n = 0; n < stringLen; n++) {
        glyphs[i].line = 0;
        glyphs[i].c = string[n];
        glyphs[i].index = FT_Get_Char_Index(font, string[n]);

        // Add kerning if present
        if (use_kerning && previous && glyphs[i].index) {
            FT_Vector  delta;
            FT_Get_Kerning(font, previous, glyphs[i].index, FT_KERNING_DEFAULT, &delta);
            penX += delta.x >> 6;
        }
        glyphs[i].pos.x = penX * 64;
        glyphs[i].pos.y = penY * 64;

        error = FT_Load_Glyph(font, glyphs[i].index, FT_LOAD_TARGET_LIGHT|FT_LOAD_FORCE_AUTOHINT);  // FT_LOAD_DEFAULT, FT_LOAD_TARGET_LIGHT
        if (error) continue;

        error = FT_Get_Glyph(font->glyph, &glyphs[i].image);
        if (error) continue;

        /* translate the glyph image now */
        FT_Glyph_Transform(glyphs[i].image, 0, &glyphs[i].pos);

        penX += font->glyph->advance.x >> 6;
        previous = glyphs[i].index;
        i++;
    }

    // i Glyphs were loaded
    const unsigned int num_glyphs = i;
    // Now we wrap the glyphs
    // Set origin to the bb min of 0th glyph
    FT_BBox  glyph_bbox, glyph_bbox_prev;
    FT_Glyph_Get_CBox(glyphs[0].image, ft_glyph_bbox_pixels, &glyph_bbox);
    glm::ivec2 origin = glm::ivec2(glyph_bbox.xMin, glyph_bbox.yMin);
    for (i = 0; i < num_glyphs; i++) {
        if (!(glyphs[i].c == '\n' || glyphs[i].c == '\r')) {
            // Get chars bbox max x
            FT_Glyph_Get_CBox(glyphs[i].image, ft_glyph_bbox_pixels, &glyph_bbox);
            // Calculate the further most x coordinate of this char
            int xMax = (padding * 2) + glyph_bbox.xMax - origin.x;
            // If char exceeds wrapping dist
            if (xMax > static_cast<int>(wrapDistance)) {
                // Find the most recent space
                int j;
                for (j = i - 1; j >= 0; j--) {
                    if (glyphs[j].c == ' ') {
                        // Mark char as not required (so we can ignore at render)
                        // and set this char to current
                        glyphs[j].c = '\n';
                        i = j;
                        break;
                    }
                }
                // Words exceeds wrap length, cancel wrapping
                if (j < 0) {
                    break;
                }
            } else {
                continue;
            }
        }
        // Move all subsequent chars to the next line
        if (i + 1 >= num_glyphs)
            continue;
        FT_Vector  newLineOffset;
        bool newline = true;
        if (glyphs[i].c == '\r')
            newline = false;
        newLineOffset.y = 0;
        /*else
            newLineOffset.y = font->height;*/
        glyph_bbox_prev = glyph_bbox;
        // If glyph[i+1] is a space, it's Cbox is empty
        // So find the next char with a Cbox and use that instead
        FT_Glyph_Get_CBox(glyphs[i + 1].image, ft_glyph_bbox_pixels, &glyph_bbox);
        if (glyph_bbox.xMin) {
            newLineOffset.x = -(glyph_bbox.xMin - origin.x) * 64;
        } else {
            while (glyphs[i + 1].c == ' ') {
                ++i;
            }
            // Advance is minimum char width
            // This special case wraps spaces properly
            // const int advance = this->font->glyph->advance.x >> 6;
            newLineOffset.x = -(glyph_bbox_prev.xMax - origin.x) * 64;
        }
        for (unsigned int j = (i + 1); j < num_glyphs; j++) {
            if (newline)
                glyphs[j].line++;
            FT_Glyph_Transform(glyphs[j].image, 0, &newLineOffset);
        }
        // Continue
    }
    // Calculate the bounding box
    FT_BBox  bbox;
    bbox.xMin = bbox.yMin = 32000;
    bbox.xMax = bbox.yMax = -32000;
    const int lineHeight = static_cast<int>((this->font->size->metrics.height >> 6));
    const int ascender = this->font->size->metrics.ascender >> 6;
    const int descender = this->font->size->metrics.descender >> 6;
    for (i = 0; i < num_glyphs; i++) {
        FT_Glyph_Get_CBox(glyphs[i].image, ft_glyph_bbox_pixels, &glyph_bbox);
        glyph_bbox.yMin = static_cast<FT_Pos>(glyph_bbox.yMax > ascender ? ((glyphs[i].line) * lineHeight * (lineSpacing + 1.0f)) - (static_cast<int>(glyph_bbox.yMax) - ascender) : ((glyphs[i].line) * lineHeight * (lineSpacing + 1.0f)));
        glyph_bbox.yMax = static_cast<FT_Pos>(glyph_bbox.yMin < descender ? ((glyphs[i].line + 1) * lineHeight * (lineSpacing + 1.0f)) + (descender - glyph_bbox.yMin) : ((glyphs[i].line + 1) * lineHeight * (lineSpacing + 1.0f)));
        if (glyph_bbox.xMin < bbox.xMin)
            bbox.xMin = glyph_bbox.xMin;

        if (glyph_bbox.yMin < bbox.yMin)
            bbox.yMin = glyph_bbox.yMin;

        if (glyph_bbox.xMax > bbox.xMax)
            bbox.xMax = glyph_bbox.xMax;

        if (glyph_bbox.yMax > bbox.yMax)
            bbox.yMax = glyph_bbox.yMax;
    }
    if (bbox.xMin > bbox.xMax) {
        fprintf(stderr, "unknown err, bounding box incorrect");
    }
    // And thus the texture size
    glm::uvec2 texDim(
        (2 * padding) + bbox.xMax - bbox.xMin,
        (2 * padding) + bbox.yMax - bbox.yMin - lineHeight*(lineSpacing));
    // Iterate chars, painting them to tex
    tex->resize(texDim);
    for (i = 0; i < num_glyphs; i++) {
        if (!(glyphs[i].c == '\n' || glyphs[i].c == '\r')) {
            error = FT_Glyph_To_Bitmap(
                &glyphs[i].image,
                printMono ? FT_RENDER_MODE_MONO : FT_RENDER_MODE_LIGHT,  // FT_RENDER_MODE_NORMAL, FT_RENDER_MODE_MONO, FT_RENDER_MODE_LIGHT
                nullptr,  // no additional translation
                1);  // destroy copy in "image"
            if (!error) {
                FT_BitmapGlyph  bit = reinterpret_cast<FT_BitmapGlyph>(glyphs[i].image);
                penX = static_cast<int>(padding) + bit->left - bbox.xMin;
                penY = static_cast<int>(padding) - bit->top + ascender + static_cast<int>(lineHeight * glyphs[i].line * (lineSpacing + 1.0f));
                // Only paint is the glyph is within bounds of the texture (report err if our maths is bad)
                if (penX >= 0 && penX + bit->bitmap.pitch <= static_cast<int>(texDim.x) && penY >= 0 && penY + static_cast<int>(bit->bitmap.rows) <= static_cast<int>(texDim.y)) {
                    if (printMono) {
                        tex->paintGlyphMono(bit->bitmap, penX, penY);
                    } else {
                        tex->paintGlyph(bit->bitmap, penX, penY);
                    }
                } else {
                    fprintf(stderr, "Skipped painting char '%c' of '%s' to avoid writing out of bounds.%i\n", glyphs[i].c, string, static_cast<int>(FT_IS_SCALABLE(this->font)));
                }
            }
        }
        FT_Done_Glyph(glyphs[i].image);
    }
    // link tex to shader
    tex->updateTex();
    // Set width
    setDimensions(texDim);
    free(glyphs);
}
void Text::setStringLen() {
    stringLen = 0;
    //  ReSharper disable once CppPossiblyErroneousEmptyStatements
    while (string[stringLen++] != '\0') { }
    stringLen--;
}
void Text::setFontHeight(unsigned int pixels, bool refreshTex) {
    if (!font) {
        fprintf(stderr, "Unable to set font height, font has not been loaded.\n");
        return;
    }
    this->fontHeight = pixels;
    FT_Error error = FT_Set_Pixel_Sizes(
        font,   /* handle to face object */
        0,      /* pixel_width           */
        this->fontHeight);/* pixel_height          */
    if (error) {
        fprintf(stderr, "An unexpected error occurred whilst setting font size: %i\n", error);
        return;
    }
    if (refreshTex)
        recomputeTex();
}
unsigned int Text::getFontHeight() const {
    return this->fontHeight;
}
void Text::setPadding(unsigned int _padding, bool refreshTex) {
    this->padding = _padding;
    if (refreshTex)
        recomputeTex();
}
unsigned int Text::getPadding() const {
    return this->padding;
}
/*
Sets whether the font should be printed with anti-aliasing
Due to a lack of fancy sub-pixel rendering techniques, smaller fonts are likely to appear clearer without anti-aliasing
@param aa The padding of overlay in pixels
@param refreshTex Whether to automatically refresh the texture
*/
void Text::setUseAA(bool aa, bool refreshTex) {
    this->printMono = !aa;
    if (refreshTex)
        recomputeTex();
}
bool Text::getUseAA() const {
    return !this->printMono;
}
void Text::setMaxWidth(unsigned int maxWidth, bool refreshTex) {
    this->wrapDistance = maxWidth;
    if (refreshTex)
        recomputeTex();
}
unsigned int Text::getMaxWidth() const {
    return this->wrapDistance;
}
void Text::setLineSpacing(float _lineSpacing, bool refreshTex) {
    this->lineSpacing = _lineSpacing;
    if (refreshTex)
        recomputeTex();
}
float Text::getLineSpacing() const {
    return this->lineSpacing;
}
void Text::setColor(glm::vec3 _color) {
    setColor(glm::vec4(_color, 1.0f));
}
void Text::setColor(glm::vec4 _color) {
    this->color = _color;
    getShaders()->addStaticUniform("_col", glm::value_ptr(this->color), 4);
}
void Text::setBackgroundColor(glm::vec3 _color) {
    setBackgroundColor(glm::vec4(_color, 1.0f));
}
void Text::setBackgroundColor(glm::vec4 _color) {
    this->backgroundColor = _color;
    getShaders()->addStaticUniform("_backCol", glm::value_ptr(this->backgroundColor), 4);
}
glm::vec4 Text::getColor() const {
    return color;
}
glm::vec4 Text::getBackgroundColor() const {
    return backgroundColor;
}
void Text::setString(const char* format, ...) {
    if (this->string) {
        free(this->string);
        this->string = nullptr;
    }
    // Create a copy of the va_list, as vsnprintf can invalidate elements of argp and find the required buffer length
    va_list argp;
    va_start(argp, format);
    va_list argpCopy;
    va_copy(argpCopy, argp);
    const int buffLen = vsnprintf(nullptr, 0, format, argpCopy) + 1;
    va_end(argpCopy);
    char* buffer = reinterpret_cast<char*>(malloc(buffLen * sizeof(char)));
    // Populate the buffer with the original va_list
    int ct = vsnprintf(buffer, buffLen, format, argp);
    if (ct >= 0) {
        // Success!
        buffer[buffLen - 1] = '\0';
    }
    this->string = buffer;
    recomputeTex();
}
Text::TextureString::TextureString()
    : Texture2D(glm::uvec2( 1, 1 ), { GL_RED, GL_RED, sizeof(unsigned char), GL_UNSIGNED_BYTE }, nullptr, Texture::DISABLE_MIPMAP | Texture::WRAP_REPEAT)
    , texture(nullptr)
    , dimensions(1, 1) {
}
void Text::TextureString::resize(const glm::uvec2 &_dimensions) {
    this->dimensions = _dimensions;
    if (texture)
        free(texture);
    texture = reinterpret_cast<unsigned char**>(malloc(sizeof(char*) * this->dimensions.y));
    texture[0] = reinterpret_cast<unsigned char*>(malloc(sizeof(char) * this->dimensions.x * this->dimensions.y));
    memset(texture[0], 0, sizeof(char)*this->dimensions.x*this->dimensions.y);
    for (unsigned int i = 1; i < this->dimensions.y; i++) {
        texture[i] = texture[i - 1] + this->dimensions.x;
    }
}
void Text::TextureString::updateTex() {
    if (!texture)
        return;
    Texture2D::resize(dimensions, texture[0]);
}
Text::TextureString::~TextureString() {
    if (texture) {
        free(texture[0]);
        free(texture);
    }
}
void Text::TextureString::paintGlyph(FT_Bitmap glyph, unsigned int penX, unsigned int penY) {
    // The static_cast on glyph.rows is required for warning supperssion with older versions of freetype such as in CenotOs 7
    for (unsigned int y = 0; y < static_cast<unsigned int>(glyph.rows); y++) {
        // src ptr maps to the start of the current row in the glyph
        unsigned char *src_ptr = glyph.buffer + y*glyph.pitch;
        // dst ptr maps to the pens current Y pos, adjusted for the current glyph row
        // unsigned char *dst_ptr = tex[penY + (glyph->bitmap.rows - y - 1)] + penX;
        unsigned char *dst_ptr = texture[penY + y] + penX;
        // copy entire row, skipping empty pixels (incase kerning causes char overlap)
        for (int x = 0; x < glyph.pitch; x++) {
            dst_ptr[x] = src_ptr[x];
        }
        // memcpy(dst_ptr, src_ptr, sizeof(unsigned char)*glyph.pitch);
    }
}
void Text::TextureString::paintGlyphMono(FT_Bitmap glyph, unsigned int penX, unsigned int penY) {
    // The static_cast on glyph.rows is required for warning supperssion with older versions of freetype such as in CenotOs 7
    for (unsigned int y = 0; y < static_cast<unsigned int>(glyph.rows); y++) {
        // src ptr maps to the start of the current row in the glyph
        unsigned char *src_ptr = glyph.buffer + y*glyph.pitch;
        // dst ptr maps to the pens current Y pos, adjusted for the current glyph row
        // unsigned char *dst_ptr = tex[penY + (glyph->bitmap.rows - y - 1)] + penX;
        unsigned char *dst_ptr = texture[penY + y] + penX;
        // copy entire row, skipping empty pixels (incase kerning causes char overlap)
        for (int x = 0; x < glyph.pitch; x++) {
            for (int j = 0; j < 8; j++)
                if (((src_ptr[x] >> (7 - j)) & 1) == 1)
                    dst_ptr[x * 8 + j] = 0xff;  // src_ptr[x];
        }
        // memcpy(dst_ptr, src_ptr, sizeof(unsigned char)*glyph.pitch);
    }
}

}  // namespace visualiser
}  // namespace flamegpu
