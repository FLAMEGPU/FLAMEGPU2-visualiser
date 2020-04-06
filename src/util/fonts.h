#ifndef SRC_UTIL_FONTS_H_
#define SRC_UTIL_FONTS_H_

#include <map>
#include <string>
#include <vector>

namespace fonts {
/**
 * Enum for generic fallback fonts, used to ensure that a font is always found
 */
enum GenericFontFamily { SANS,
                         SERIF,
                         CURSIVE,
                         MONOSPACE };

/**
 * Find a single font, or use a fallback font
 * @param fontname string fontname for the desirable font
 * @param generic enum specifying a fallback font type in case the target font could not be found.
 * @return path to font to use, or empty string if nothing found (if no system fonts installed)
 */
std::string findFont(std::string fontname, const GenericFontFamily generic);

/**
 * Find a font from an ordered list of desired fonts, or use a fallback font
 * @param fontnames vector of string fontnames
 * @param generic enum specifying a fallback font type in case the target font could not be found.
 * @return path to font to use, or empty string if nothing found (if no system fonts installed)
 */
std::string findFont(std::vector<std::string> fontnames, const GenericFontFamily generic);

} // namespace fonts

#endif // SRC_UTIL_FONTS_H_
