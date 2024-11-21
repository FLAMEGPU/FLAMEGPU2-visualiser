#include "flamegpu/visualiser/ui/SplashScreen.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "flamegpu/visualiser/ui/Sprite2D.h"
#include "flamegpu/visualiser/ui/Text.h"
#include "flamegpu/visualiser/util/fonts.h"
#include "flamegpu/visualiser/util/Resources.h"

namespace flamegpu {
namespace visualiser {

SplashScreen::SplashScreen(const glm::vec3& textColor, const std::string& message, bool isPython)
    : logo_offset(0)
    , text_offset(0) {
    int logo_height = 0;
    try {
        logo = std::make_shared<Sprite2D>(Texture2D::load(isPython ? "resources/pyflamegpu.png" : "resources/flamegpu.png"));
        logo_height = logo->getHeight();
    } catch(...) {
        // Fail silently, branding is not required
        fprintf(stderr, "Failed to load splash screen logo: %s\n", Resources::locateFile(isPython ? "resources/pyflamegpu.png" : "resources/flamegpu.png").c_str());
    }
    text = std::make_shared<Text>("Loading...", 45, textColor, fonts::findFont({ "Arial" }, fonts::GenericFontFamily::SANS).c_str());
    // Calculate offsets for vertical stacking
    const int PADDING = 10;
    const int FULL_HEIGHT = logo_height + PADDING + text->getHeight();
    const int MID_HEIGHT = FULL_HEIGHT / 2;
    logo_offset.y = MID_HEIGHT - (logo_height / 2);
    text_offset.y = MID_HEIGHT - (logo_height + PADDING + (text->getHeight() / 2));
}
std::vector<SplashScreen::OverlayItem> SplashScreen::getOverlays() const {
    if (logo) {
        return std::vector<OverlayItem>{OverlayItem{ logo, logo_offset, 0 }, OverlayItem{ text, text_offset, 0 }};
    } else {
        return std::vector<OverlayItem>{OverlayItem{ text, text_offset, 0 }};
    }
}

}  // namespace visualiser
}  // namespace flamegpu
