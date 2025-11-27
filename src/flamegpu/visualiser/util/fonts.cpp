#include "flamegpu/visualiser/util/fonts.h"
#include "flamegpu/visualiser/util/VisException.h"

// Linux implementation
#if defined(__GNUC__) || defined(__clang__)

#include <stdio.h>
#include <fontconfig/fontconfig.h>
#include <string>

namespace flamegpu {
namespace visualiser {

namespace fonts {
// private namespace
namespace {
/**
 * Get the name of a generic font from the enum.
 * @param generic font family to search for
 * @return name of the generic font.
 */
std::string genericFontName(const GenericFontFamily generic) {
    // @note - doesn't seem to be an actual way of detecting generic fonts through directwrite.
    switch (generic) {
    default:
    case SANS:
        return "sans-serif";
        break;
    case SERIF:
        return "serif";
        break;
    case CURSIVE:
        return "cursive";
        break;
    case MONOSPACE:
        return "monospace";
        break;
    }
}

/**
 * Construct a query string for fontconfig FCNameParse - a comma separated list of strings
 * @param fontNames a list of font names
 * @param generic the generic font family
 * @return a std::string containing the comma separated font names, including  
 */
std::string fontQueryString(std::initializer_list<const char *> fontNames, const GenericFontFamily generic) {
    std::string query = "";
    // Add each non-empty string and a trailing comma to the list.
    for (auto fontName_cstr : fontNames) {
        std::string fontName = fontName_cstr;
        if (fontName.length() > 0) {
            query += fontName + ",";
        }
    }
    // Attach the generic family name
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
    FcConfig* config = FcInitLoadConfigAndFonts();

    // Construct a pattern searching for the desired font
    FcPattern* pat = FcNameParse((const FcChar8*)(commaSeparatedfonts.c_str()));

    // Apply fontconfig substitution.
    FcConfigSubstitute(config, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    // Apply the fontconfig matching
    FcResult res;
    FcPattern* font = FcFontMatch(config, pat, &res);

    // If we have a result
    if (font) {
        // Extract the path to the font for later
        FcChar8* file = NULL;
        if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
            fontpath = std::string(reinterpret_cast<char *>(file));
        }
        FcPatternDestroy(font);
    }
    // Tidy up.
    FcPatternDestroy(pat);

    // Destroy the FcConfig - this will force a re-initialisation on subsequent calls to fontSearch, but prevent leakage
    FcConfigDestroy(config);
    config = nullptr;

    // Fully clean up font config - this should probably be in a called-once function rather than on each invocation
    FcFini();

    // Return the path to the desired font.
    return fontpath;
}
}  // Anonymous namespace

/**
 * Find a font from an ordered list of desired fonts, or use a fallback font
 * @param fontnames a list of fontnames as constant character arrays
 * @param generic enum specifying a fallback font type in case the target font could not be found.
 * @return path to font to use, or empty string if nothing found (if no system fonts installed)
 * 
 * @note this does not use a list of std::strings as this complicated initialisation.
 */
std::string findFont(std::initializer_list<const char *> fontNames, const GenericFontFamily generic) {
    // Construct the fontName search
    std::string query = fontQueryString(fontNames, generic);

    // Apply the query
    std::string fontpath = fontSearch(query);

    // if none were found, an error has occured. @todo
    if (fontpath.length() == 0) {
        THROW FontLoadingError("Fontconfig Font Loading Error at %s::%u", __FILE__, __LINE__);
    }
    // Return the path to the font.
    return fontpath;
}


}  // namespace fonts

}  // namespace visualiser
}  // namespace flamegpu


// Windows implementation using directwrite.
#elif defined(_MSC_VER)

// https://docs.microsoft.com/en-us/windows/win32/api/dwrite/nn-dwrite-idwritefontcollection
#include <dwrite.h>

#include <string>

namespace flamegpu {
namespace visualiser {
namespace fonts {

// anonymous namespace to emulate private:
namespace {

/**
 * Find the path to a single named font on windows using directwrite
 * @param fontName the name of the font. I.e. Arial, Papyrus
 * @return full path to the font, if found. Empty string if not.
 */
std::string fontSearch(std::string fontName) {
    // Initialise default empty string return value.
    std::string retval = std::string("");

    // Get a the directwrite factory object
    IDWriteFactory* directWriteFactory = NULL;
    HRESULT dwriteResult = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown * *>(&directWriteFactory));

    if (!SUCCEEDED(dwriteResult)) {
        THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
    }

    // Get the font collection for the system
    IDWriteFontCollection* fontCollection = NULL;
    dwriteResult = directWriteFactory->GetSystemFontCollection(&fontCollection);
    if (!SUCCEEDED(dwriteResult)) {
        THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
    }

    // Search for a family name within a collection.
    unsigned int targetLength = MultiByteToWideChar(CP_UTF8, 0, fontName.c_str(), -1, NULL, 0);
    WCHAR* targetFamilyName = new WCHAR[targetLength];
    MultiByteToWideChar(CP_UTF8, 0, fontName.c_str(), -1, targetFamilyName, targetLength);

    UINT32 matchIndex = 0;
    BOOL targetFound = false;
    dwriteResult = fontCollection->FindFamilyName(targetFamilyName, &matchIndex, &targetFound);
    if (!SUCCEEDED(dwriteResult)) {
        delete targetFamilyName;
        THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
    }
    delete targetFamilyName;
    // If the target was found, select it via indexing.
    if (targetFound) {
        IDWriteFontFamily* fontFamily = NULL;

        // Get the font family.
        dwriteResult = fontCollection->GetFontFamily(matchIndex, &fontFamily);
        if (!SUCCEEDED(dwriteResult)) {
            THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
        }

        UINT32 fontCount = fontFamily->GetFontCount();
        for (UINT32 j = 0; j < fontCount; j++) {
            IDWriteFont * font = NULL;
            dwriteResult = fontFamily->GetFont(j, &font);
            if (!SUCCEEDED(dwriteResult)) {
                THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
            }

            // Get the direct write font face
            IDWriteFontFace *fontFace = NULL;
            dwriteResult = font->CreateFontFace(&fontFace);
            if (!SUCCEEDED(dwriteResult)) {
                THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
            }

            // Find how many files are associated with the font face.
            UINT32 numFontFaceFiles = 0;
            fontFace->GetFiles(&numFontFaceFiles, NULL);
            if (!SUCCEEDED(dwriteResult)) {
                THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
            }

            if (numFontFaceFiles > 0) {
                // Get the associated font files.
                IDWriteFontFile *fontFaceFiles = NULL;
                fontFace->GetFiles(&numFontFaceFiles, &fontFaceFiles);
                if (!SUCCEEDED(dwriteResult)) {
                    THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
                }

                // Get the file loader for the first file.
                IDWriteFontFileLoader *fontFileLoader = NULL;
                IDWriteLocalFontFileLoader *localFontFileLoader = NULL;
                dwriteResult = fontFaceFiles[0].GetLoader(&fontFileLoader);
                if (!SUCCEEDED(dwriteResult)) {
                    THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
                }
                // Ensure it is a local file
                dwriteResult = fontFileLoader->QueryInterface(__uuidof(IDWriteLocalFontFileLoader), reinterpret_cast<void **>(&localFontFileLoader));
                if (SUCCEEDED(dwriteResult)) {
                    // Get the path to the file.
                    const void *referenceKey = NULL;
                    UINT32 referenceKeySize = 0;
                    dwriteResult = fontFaceFiles[0].GetReferenceKey(&referenceKey, &referenceKeySize);
                    if (!SUCCEEDED(dwriteResult)) {
                        THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
                    }
                    UINT32 wFilePathLength = 0;
                    dwriteResult = localFontFileLoader->GetFilePathLengthFromKey(referenceKey, referenceKeySize, &wFilePathLength);
                    if (!SUCCEEDED(dwriteResult)) {
                        THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
                    }

                    WCHAR *wFilePath = nullptr;
                    wFilePath = new WCHAR[wFilePathLength + 1];
                    dwriteResult = localFontFileLoader->GetFilePathFromKey(referenceKey, referenceKeySize, wFilePath, wFilePathLength + 1);
                    if (!SUCCEEDED(dwriteResult)) {
                        if (wFilePath) {
                            delete wFilePath;
                        }
                        THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
                    }

                    // Convert the WCHAR filepath to a char*
                    int filePathLength = WideCharToMultiByte(CP_UTF8, 0, wFilePath, -1, NULL, 0, NULL, NULL);
                    char *filePath = new char[filePathLength];
                    WideCharToMultiByte(CP_UTF8, 0, wFilePath, -1, filePath, filePathLength, NULL, NULL);

                    // Put it into a std string to return.
                    retval = std::string(filePath);

                    delete filePath;
                    filePath = nullptr;
                    delete wFilePath;
                    wFilePath = nullptr;

                    // Also break out the loop so that the first matching filepath is used.
                    break;
                }

                if (fontFileLoader) {
                    fontFileLoader->Release();
                    fontFileLoader = nullptr;
                }
                if (localFontFileLoader) {
                    localFontFileLoader->Release();
                    localFontFileLoader = nullptr;
                }
            }
        }

        if (fontFamily) {
            fontFamily->Release();
            fontFamily = nullptr;
        }
    }

    if (fontCollection) {
        fontCollection->Release();
        fontCollection = nullptr;
    }
    if (directWriteFactory) {
        directWriteFactory->Release();
        directWriteFactory = nullptr;
    }
    return retval;
}

/*
    * Define default font names for Generic families. These should exist on (almost) all windows systems.
    */
std::string genericFontName(GenericFontFamily generic) {
    // @note - doesn't seem to be an actual way of detecting generic fonts through directwrite.
    switch (generic) {
        default:
        case SANS:
            return "Arial";
            break;
        case SERIF:
            return "Times New Roman";
            break;
        case CURSIVE:
            return "Comic Sans MS";
            break;
        case MONOSPACE:
            return "Courier New";
            break;
    }
}

/*
    * Define the name of a fallback font.
    * @todo - this should probably actually just be a selected font which meets some properties, rather than a specific named value.
    */
const char* FALLBACK_FONTNAME = "Arial";
}  // Anonymous namespace

/**
 * Find a font from an ordered list of desired fonts, or use a fallback font
 * @param fontNames a list of fontnames as constant character arrays
 * @param generic enum specifying a fallback font type in case the target font could not be found.
 * @return path to font to use, or empty string if nothing found (if no system fonts installed)
 * 
 * @note this does not use a list of std::strings as this complicated initialisation.
 */
std::string findFont(std::initializer_list<const char *> fontNames, const GenericFontFamily generic) {
    // For each font in the list, search for it. Return if found.
    for (auto _fontName : fontNames) {
        std::string fontName(_fontName);
        if (fontName.length() > 0) {
            std::string fontpath = fontSearch(fontName);
            if (fontpath.length() > 0) {
                return fontpath;
            }
        }
    }
    // Try the _generic font
    std::string genericFontpath = fontSearch(genericFontName(generic));
    if (genericFontpath.length() > 0) {
        return genericFontpath;
    } else {
        std::string fallbackFontpath = fontSearch(FALLBACK_FONTNAME);
        if (fallbackFontpath.length() > 0) {
            return fallbackFontpath;
        }
    }
    // Otherwise we need to bail out  without any fonts.
    THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
}

}  // namespace fonts
}  // namespace visualiser
}  // namespace flamegpu

#endif


