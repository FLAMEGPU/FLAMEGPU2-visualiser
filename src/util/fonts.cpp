#include "fonts.h"
#include "util/VisException.h"

// Linux implementation
#if defined(__GNUC__) || defined(__clang__)

#include <stdio.h>
#include <fontconfig/fontconfig.h>

namespace fonts {
    // private namespace
    namespace {
        /**
             * Map of the generic font family enum to font names
             */
        const std::map<GenericFontFamily, const char*> genericFontNameMap = {
            {SANS, "sans-serif"},
            {SERIF, "serif"},
            {MONOSPACE, "monospace"},
            {CURSIVE, "cursive"} };
        /**
             *  Generic fallback font name, in case the genericFontNameMap is incomplete for some reason
             */
        const char* FALLBACK_FONTNAME = "sans-serif";

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
            }
            else {
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
                    fontpath = std::string((char*)file);
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
        if (!fontpath.length() == 0) {
            THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__); // @todo exception.
        }
        // Return the path to the font.
        return fontpath;
    }

    std::string findFont(std::string fontName, const GenericFontFamily generic) {
        // Attempt to find a font. If it doesn't match, use the fallback.
        std::string query = fontQueryString(fontName, generic);
        std::string fontpath = fontSearch(query);
        if (!fontpath.length() == 0) {
            THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__); // @todo exception.
        }
        // Return the path to the font.
        return fontpath;
    }

}

// Windows implementation
#elif defined(_MSC_VER)

#include <dwrite.h>

namespace fonts {

// private stuff
namespace {

    const std::map<GenericFontFamily, const char*> genericFontNameMap = {
        {SANS, "sans-serif"},
        {SERIF, "serif"},
        {MONOSPACE, "monospace"},
        {CURSIVE, "cursive"} };
    /**
    *  Generic fallback font name, in case the genericFontNameMap is incomplete for some reason
    */
    const char* FALLBACK_FONTNAME = "sans-serif";

    // https://docs.microsoft.com/en-us/windows/win32/api/dwrite/nn-dwrite-idwritefontcollection

    std::string fontSearch(std::string fontName) {

        std::string fontpath = std::string("");

        // Get a the directwrite factory object
        IDWriteFactory* directWriteFactory = NULL;
        HRESULT dwriteResult = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown * *>(&directWriteFactory)
        );

        if (!SUCCEEDED(dwriteResult)) {
            THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
        }

        // Get the font collection for the system
        IDWriteFontCollection* fontCollection = NULL;
        dwriteResult = directWriteFactory->GetSystemFontCollection(&fontCollection);
        if (!SUCCEEDED(dwriteResult)) {
            THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
        }

        // Get the number of font families in the collection.
        UINT32 familyCount = fontCollection->GetFontFamilyCount();


        // Iterate the families
        for (UINT32 i = 0; i < familyCount; i++) {
            IDWriteFontFamily* fontFamily = NULL;

            // Get the font family.
            dwriteResult = fontCollection->GetFontFamily(i, &fontFamily);
            if (!SUCCEEDED(dwriteResult)) {
                THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
            }

            UINT32 fontCount = fontFamily->GetFontCount();
            for(UINT32 j = 0; j < fontCount; j++){
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

                // Find how many files are assocaited with the font face.
                UINT32 numFontFaceFiles = 0;
                fontFace->GetFiles(&numFontFaceFiles, NULL);
                if (!SUCCEEDED(dwriteResult)) {
                    THROW FontLoadingError("Windows Font Loading Error at %s::%u", __FILE__, __LINE__);
                }
                
                if(numFontFaceFiles > 0){
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
                    dwriteResult = fontFileLoader->QueryInterface(__uuidof(IDWriteLocalFontFileLoader), (void **)&localFontFileLoader);
                    if(SUCCEEDED(dwriteResult)){
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

						/*char* postscriptName = getString(font, DWRITE_INFORMATIONAL_STRING_POSTSCRIPT_NAME);
						char* family = getString(font, DWRITE_INFORMATIONAL_STRING_WIN32_FAMILY_NAMES);
						char* style = getString(font, DWRITE_INFORMATIONAL_STRING_WIN32_SUBFAMILY_NAMES);*/
						

						printf("filePath %s\n", filePath);

						/*delete postscriptName;
						delete family;
						delete style;*/


						delete filePath;
						filePath = nullptr;
						delete wFilePath;
						wFilePath = nullptr;
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
		exit(1);
        return fontpath;

    }

    std::string genericFontName(GenericFontFamily generic) {
        // @todo - map.
        return std::string("Arial");
    }

}

// Public things.

std::string findFont(std::vector<std::string> fontNames, const GenericFontFamily generic) {

    // For each font in the vector, search for it. Return if found.
    for (auto fontName : fontNames) {
        if (fontName.length() > 0) {
            std::string fontpath = fontSearch(fontName);
            if (fontpath.length() > 0) {
                return fontpath;
            }
        }
    }
    // Try the generic font
    std::string genericFontpath = fontSearch(genericFontName(generic));
    if (genericFontpath.length() > 0) {
        return genericFontpath;
    }
    else {
        std::string fallbackFontpath = fontSearch(FALLBACK_FONTNAME);
        if (fallbackFontpath.length() > 0) {
            return fallbackFontpath;
        }
    }

    // Otherwise we need to bail out  without any fonts.
    // @todo raise an exception
    return "";
}

std::string findFont(std::string fontName, const GenericFontFamily generic) {
    
    // Try to find the font specified.
    if (fontName.length() > 0) {
        std::string fontpath = fontSearch(fontName);
        if (fontpath.length() > 0) {
            return fontpath;
        }
    }

    // Try the generic font
    std::string genericFontpath = fontSearch(genericFontName(generic));
    if (genericFontpath.length() > 0) {
        return genericFontpath;
    }
    else {
        std::string fallbackFontpath = fontSearch(FALLBACK_FONTNAME);
        if (fallbackFontpath.length() > 0) {
            return fallbackFontpath;
        }
    }

    // Otherwise we need to bail out  without any fonts.
    // @todo raise an exception
    return "";
}
} // namespace fonts

#endif


