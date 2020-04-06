#include "fonts.h"

namespace fonts {

// Linux implementation
#if defined(__GNUC__) || defined(__clang__)

#include <stdio.h>
#include <fontconfig/fontconfig.h>

// private namespace
namespace {
/**
     * Map of the generic font family enum to font names
     */
const std::map<GenericFontFamily, const char *> genericFontNameMap = {
    {SANS, "sans-serif"},
    {SERIF, "serif"},
    {MONOSPACE, "monospace"},
    {CURSIVE, "cursive"}};
/**
     *  Generic fallback font name, in case the genericFontNameMap is incomplete for some reason
     */
const char *FALLBACK_FONTNAME = "sans-serif";

/**
     * Get the name of a generic font from the enum.
     * @param generic font family to search for
     * @return name of the generic font.
     */
std::string genericFontName(const GenericFontFamily generic) {
    // Get the font name from the map.
    std::string fontName = "";
    auto it = genericFontNameMap.find(generic);
    if (it != genericFontNameMap.end()) {
        // If the generic key exists in the map (it should) use the relevant string.
        fontName = it->second;
    } else {
        // use the fallback font if could not find the key in the generic font map.
        fontName = std::string(FALLBACK_FONTNAME);
    }
    return fontName;
        }

        std::string fontQueryStringVector(std::vector<std::string> fontNames, const GenericFontFamily generic) {
            std::string query = "";
            for (auto fontName : fontNames) {
                if (fontName.length() > 0) {
                    query += fontName + ",";
                }
            }

            query += genericFontName(generic);
            return query;
        }
        std::string fontQueryString(std::string fontName, const GenericFontFamily generic) {
            std::string query = "";
            if (fontName.length() > 0) {
                query += fontName + ",";
            }
            query += genericFontName(generic);
            return query;
        }

        /**
     * Find a single font from a string
     * @param comma separated list of fontNames to search for.
     * @return string of path to font file, or empty string if un found
     */
        std::string fontSearch(const std::string commaSeparatedfonts) {
            std::string fontpath = "";

            // Initialise fontconfig
            FcConfig *config = FcInitLoadConfigAndFonts();

            // Construct a pattern searching for the desired font
            FcPattern *pat = FcNameParse((const FcChar8 *)(commaSeparatedfonts.c_str()));

            // Apply fontconfig substitution.
            FcConfigSubstitute(config, pat, FcMatchPattern);
            FcDefaultSubstitute(pat);
            // Apply the fontconfig matching
            FcResult res;
            FcPattern *font = FcFontMatch(config, pat, &res);

            // If we have a result
            if (font) {
                // Extract the path to the font for later
                FcChar8 *file = NULL;
                if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
                    fontpath = std::string((char *)file);
                }
                FcPatternDestroy(font);
            }
            // Tidy up.
            FcPatternDestroy(pat);

            // Return the path to the desired font.
            printf("%s ? %s\n", commaSeparatedfonts.c_str(), fontpath.c_str());
            return fontpath;
        }
    }

    std::string findFont(std::vector<std::string> fontNames, const GenericFontFamily generic) {

        // Construct the fontName search
        std::string query = fontQueryStringVector(fontNames, generic);

        // Apply the query
        std::string fontpath = fontSearch(query);

        // if none were found, an error has occured. @todo
        if (fontpath.length() == 0) {
            exit(1); // @todo exception.
        }
        // Return the path to the font.
        return fontpath;
    }

    std::string findFont(std::string fontName, const GenericFontFamily generic) {
        // Attempt to find a font. If it doesn't match, use the fallback.
        std::string query = fontQueryString(fontName, generic);
        std::string fontpath = fontSearch(query);
        if (fontpath.length() == 0) {
            exit(1); // @todo exception.
        }
        // Return the path to the font.
        return fontpath;
    }

// Windows implementation
#elif defined(_MSC_VER)

#endif

} // namespace fonts
