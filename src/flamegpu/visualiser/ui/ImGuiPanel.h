#ifndef SRC_FLAMEGPU_VISUALISER_UI_IMGUIPANEL_H_
#define SRC_FLAMEGPU_VISUALISER_UI_IMGUIPANEL_H_

#include <memory>
#include <string>
#include <map>
#include <list>

#include "flamegpu/visualiser/config/PanelConfig.h"
#include "flamegpu/visualiser/ui/Overlay.h"

namespace flamegpu {
namespace visualiser {

class Visualiser;

/**
 * Class for rendering all ImGui user interfaces to the HUD
 *
 * ImGui draws to the whole screen space, so we handle them all at the same time for ease of input handling
 */
class ImGuiPanel : public Overlay {
 public:
    /**
     * Default constructor, currently creates a debugging panel thing
     * @param cfgs Map of configs specifying how they should be constructed
     * @param _vis Handle to the visualiser so that data can be probed by the debug panel
     */
    explicit ImGuiPanel(const std::map<std::string, std::shared_ptr<PanelConfig>> &cfgs, const Visualiser& _vis);
    /**
     * Only resets first_render flag, as ImGui runs in immediate mode
     */
    void reload() override;
    /**
     * Renders the desired panels using ImGui
     * Arguments are ignored, ImGui detects and sets these internally
     */
    void render(const glm::mat4*, const glm::mat4*, GLuint) override;
    /**
     * This must be called for every environment property stored within configs
     * It provides the host cache pointer and constant status of each property
     * @param name Name of the environment property
     * @param ptr Pointer to the environment properties data within the host cache
     * @param is_const True if the environment property should not be changed
     * @param is_array True if the environment property's name should reflect that it's an array within the UI
     */
    void registerProperty(const std::string &name, void *ptr, bool is_const, bool is_array);
    /**
     * Toggle visibility of the debug menu
     * @note If debug menu is made visible, the user specified panels are hidden
     */
    void toggleDebugMenuVisible() { debug_menu_visible = !debug_menu_visible; if (debug_menu_visible) { ui_visible = false; }}
    /**
     * Toggle visibility of the user specified panels
     * @note If the user specified panels are made visible, the debug menu is hidden
     */
    void toggleUIVisible() { ui_visible = !ui_visible; if (ui_visible) { debug_menu_visible = false; }}

 private:
     /**
      * If true render() triggers drawDebugPanel()
      */
    bool debug_menu_visible = false;
    /**
     * If true render() triggers drawPanel()
     */
    bool ui_visible = true;
    /**
     * Calls ImGui to draw the user specified panels from configs
     */
    void drawPanel();
    /**
     * Calls ImGui to draw the debug panel
     */
    void drawDebugPanel() const;
    /**
     * Causes panels to be automatically positioned
     * Takes 2 frames to process, due to auto sizing
     */
    unsigned char first_render;
    /**
     * A copy of the panel configurations passed to the constructor
     * These are maintaned as they must be passed to ImGui prior to each frame being rendered
     */
    std::list<PanelConfig> configs;
    /**
     * Used by drawDebugPanel() to access visualisation properties
     */
    const Visualiser &vis;
};

}  // namespace visualiser
}  // namespace flamegpu
#endif  // SRC_FLAMEGPU_VISUALISER_UI_IMGUIPANEL_H_
