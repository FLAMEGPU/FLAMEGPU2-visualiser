#ifndef SRC_FLAMEGPU_VISUALISER_UI_SPLASHSCREEN_H_
#define SRC_FLAMEGPU_VISUALISER_UI_SPLASHSCREEN_H_

#include <memory>
#include <vector>
#include <string>

#include "flamegpu/visualiser/ui/Overlay.h"

namespace flamegpu {
namespace visualiser {

class Sprite2D;
class Text;

/**
 * Displays the logo and a loading notice
 * This should be placed in the center of the UI
 */
class SplashScreen : public OverlayGroup {
 public:
    /**
     * @param textColor The color of the message
     * @param message The displayed message
     * @param isPython If true, use alt python logo
     */
    explicit SplashScreen(const glm::vec3 &textColor, const std::string &message = "Loading...", bool isPython = false);
    /**
     * Return a copy of the collection of overlays and their offsets
     */
    std::vector<OverlayItem> getOverlays() const override;

 private:
    glm::ivec2 logo_offset, text_offset;
    std::shared_ptr<Sprite2D> logo;
    std::shared_ptr<Text> text;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // SRC_FLAMEGPU_VISUALISER_UI_SPLASHSCREEN_H_
