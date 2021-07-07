#ifndef SRC_FLAMEGPU_VISUALISER_UTIL_FONTS_H_
#define SRC_FLAMEGPU_VISUALISER_UTIL_FONTS_H_

#include <map>
#include <string>
#include <vector>

namespace flamegpu {
namespace visualiser {
namespace fonts {
/**
 * Enum for generic fallback fonts, used to ensure that a font is always found
 */
enum GenericFontFamily { SANS, SERIF, CURSIVE, MONOSPACE };

/**
 * Find a font from an ordered list of desired fonts, or use a fallback font
 * @param fontnames a list of fontnames as constant character arrays
 * @param generic enum specifying a fallback font type in case the target font could not be found.
 * @return path to font to use, or empty string if nothing found (if no system fonts installed)
 * 
 * @note this does not use a list of std::strings as this complicated initialisation.
 */
std::string findFont(std::initializer_list<const char *> fontnames, const GenericFontFamily generic);

}  // namespace fonts
}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_UTIL_FONTS_H_
