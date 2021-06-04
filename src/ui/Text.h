#ifndef SRC_UI_TEXT_H_
#define SRC_UI_TEXT_H_

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <memory>

#include "Overlay.h"

namespace flamegpu {
namespace visualiser {

/**
 * Class for rendering strings to screen.
 * Windows stores font name-file name mappings in HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts
 * Those installed fonts are then stored in C:/Windows/Fonts/
 */
class Text : public Overlay {
    class TextureString : public Texture2D {
     public:
        /**
         * Creates a new TextureString which represents the texture holding the glyphs of the string
         */
        TextureString();
        /**
         * Frees the texture's data
         */
        ~TextureString();
        /**
         * Resizes the texture
         * @param _dimensions The dimensions of the texture to be created
         */
        void resize(const glm::uvec2 &_dimensions) override;
        using RenderTarget::resize;
        /**
         * Paints a single character glyph to the texture at the specified location from a 1-byte texture
         * @param penX The x coordinate that the top-left corner of the glyphs bounding-box maps to within the texture
         * @param penY The y coordinate that the top-left corner of the glyphs bounding-box maps to within the texture
         */
        void paintGlyph(FT_Bitmap glyph, unsigned int penX, unsigned int penY);
        /**
         * Paints a single character glyph to the texture at the specified location from a 1-bit mono texture
         * @param penX The x coordinate that the top-left corner of the glyphs bounding-box maps to within the texture
         * @param penY The y coordinate that the top-left corner of the glyphs bounding-box maps to within the texture
         */
        void paintGlyphMono(FT_Bitmap glyph, unsigned int penX, unsigned int penY);
        /**
         * Updates the GL texture to match the painted texture
         */
        void updateTex();

     private:
        unsigned char **texture;
        glm::uvec2 dimensions;
    };

 public:
    /**
     * Creates a text overlay with the provided string
     * @param string The text to be included in the overlay
     * @param fontHeight The pixel height of the text
     * @param color The rgb(0-1) color of the font
     * @param fontFile The path to the desired font
     * @param faceIndex The face within the font file to be used (most likely 0)
     */
    Text(const char *string, unsigned int fontHeight, glm::vec3 color, char const *fontFile = nullptr, unsigned int faceIndex = 0);
    /**
     * Creates a text overlay with the provided string
     * @param string The text to be included in the overlay
     * @param fontHeight The pixel height of the text
     * @param color The rgba(0-1) color of the font
     * @param fontFile The path to the desired font
     * @param faceIndex The face within the font file to be used (most likely 0)
     */
    explicit Text(const char *string, unsigned int fontHeight = 20, glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), char const *fontFile = nullptr, unsigned int faceIndex = 0);
    /**
     * Deallocates the loaded font and other allocated elements
     */
    virtual ~Text();
    /**
     * Repaints the text to a texture, according to the provided parameters
     */
    void reload() override;
    /**
     * Sets the color of the font
     * @param color The RGB(0-1) font color to be used when rendering the text
     */
    void setColor(glm::vec3 color);
    /**
     * Sets the color of the font
     * @param color The RGBA(0-1) font color to be used when rendering the text
     * @note If an alpha value of -1 is used, you can achieve transparent text on a background
     */
    void setColor(glm::vec4 color);
    /**
     * Sets the color of the texture
     * @param color The RGB(0-1) background color to be used when rendering the text
     */
    void setBackgroundColor(glm::vec3 color);
    /**
     * Sets the color of the texture
     * @param color The RGBA(0-1) background color to be used when rendering the text
     */
    void setBackgroundColor(glm::vec4 color);
    /**
     * Sets the height of the font
     * @param pixels The height of the font in pixels
     * @param refreshTex Whether to automatically refresh the texture
     */
    void setFontHeight(unsigned int pixels, bool refreshTex = true);
    /**
     * Returns the currently stored font height (measured in pixels)
     * @return The stored font height
     * @note If refreshTex was set to false when the height was updated, this may not reflect the rendered text
     */
    unsigned int getFontHeight() const;
    /**
     * Sets the padding (distance between text bounding box and teture bounding box) of the overlay
     * @param padding The padding of overlay in pixels
     * @param refreshTex Whether to automatically refresh the texture
     */
    void setPadding(unsigned int padding, bool refreshTex = true);
    /**
     * Returns the currently stored padding (measured in pixels)
     * @return The stored padding
     * @note If refreshTex was set to false when the padding was updated, this may not reflect the rendered text
     */
    unsigned int getPadding() const;
    /**
     * Sets the maximum width of the texture, if text extends past this it will be wrapped
     * @param maxWidth The max width of the texture in pixels
     * @param refreshTex Whether to automatically refresh the texture
     */
    void setMaxWidth(unsigned int maxWidth, bool refreshTex = true);
    /**
     * Returns the maximum width of the texture, if text extends past this it will be wrapped
     * @return The maximum width of the texture
     * @note If refreshTex was set to false when the max width was updated, this may not reflect the rendered text
     */
    unsigned int getMaxWidth() const;
    /**
     * Sets the line spacing, this is measured as a proporition of the line height (which may include space or not, dependent on font)
     * @param lineSpacing The line spacing, recommended values are -0.2-0.0
     * @param refreshTex Whether to automatically refresh the texture
     */
    void setLineSpacing(float lineSpacing, bool refreshTex = true);
    /**
     * Returns whether the font is being rendered with anti-aliasing
     * @return The stored aliasing state
     * @note If refreshTex was set to false when the padding was updated, this may not reflect the rendered text
     */
    bool getUseAA() const;
    void setUseAA(bool useAA, bool refreshTex = true);
    /**
     * Returns the line spacing of the text
     * @return The line spacing of the text
     * @note If refreshTex was set to false when the line spacing was updated, this may not reflect the rendered text
     */
    float getLineSpacing() const;
    /**
     * Returns the font color used to render the text
     */
    glm::vec4 getColor() const;
    /**
     * Returns the background color of the texture
     */
    glm::vec4 getBackgroundColor() const;
    /**
     * Updates the string using a string format
     * @param fmt Matches those used by functions such as printf(), sprintf() etc
     * @note This function simply wraps snprintf() for convenience
     * @note This function will always refresh the texture
     */
    void setString(const char* format, ...);

 private:
    bool printMono;
    unsigned int padding;
    float lineSpacing;  // Line spacing calculated as a percentage of font Height
    glm::vec4 color;
    glm::vec4 backgroundColor;
    /**
     * Repaints the text to a texture, according to the provided parameters
     * @note Based on http:// www.freetype.org/freetype2/docs/tutorial/step2.html
     */
    void recomputeTex();
    /**
     * Internal method used to update the variable stringLen according to the length of string
     */
    void setStringLen();
    FT_Library  library;
    FT_Face     font;
    char *string;
    unsigned int stringLen;
    unsigned int fontHeight;
    unsigned int wrapDistance;
    std::shared_ptr<TextureString> tex;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_UI_TEXT_H_
