#ifndef INCLUDE_FLAMEGPU_VISUALISER_CONFIG_PANELCONFIG_H_
#define INCLUDE_FLAMEGPU_VISUALISER_CONFIG_PANELCONFIG_H_

#include <list>
#include <memory>
#include <string>

#include "ImGuiWidgets.h"

namespace flamegpu {
namespace visualiser {

/**
 * Represents the user-specified elements to be shown within an ImGui panel as part of the visualisation's UI
 */
struct PanelConfig {
    /**
     * Constructor, only a name is specified, elements can be added later
     * @_title Name of the panel to be displayed in the title bar
     */
    explicit PanelConfig(const std::string &_title)
        : title(_title) { }
    /**
     * Copy constructor
     * @other Other PanelConfig to be copied
     */
    explicit PanelConfig(const PanelConfig& other)
        : title(other.title) {
        for (const auto &e : other.ui_elements) {
            this->ui_elements.push_back(e->clone());
        }
    }
    /**
     * Name of the panel
     * This will normally be shown in the title_bar at the top of the panel
     */
    std::string title;
    /**
     * Elements will be applied in order
     */
    std::list<std::unique_ptr<PanelElement>> ui_elements;
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // INCLUDE_FLAMEGPU_VISUALISER_CONFIG_PANELCONFIG_H_
