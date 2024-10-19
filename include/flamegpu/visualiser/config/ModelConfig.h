#ifndef INCLUDE_FLAMEGPU_VISUALISER_CONFIG_MODELCONFIG_H_
#define INCLUDE_FLAMEGPU_VISUALISER_CONFIG_MODELCONFIG_H_

#include <string>
#include <list>
#include <map>
#include <memory>

#include "LineConfig.h"
#include "PanelConfig.h"

namespace flamegpu {
namespace visualiser {

/**
 * This class holds the common components for a visualisation
 * @note I had wanted to avoid including C++ types in the Visualisation interface Lib,
 * however achieving some of the dynamic elements without would require inclusion of a C list implementation/similar.
 * As such, it is important the library is built with same compiler as main FGPU2 lib.
*/
struct ModelConfig {
    friend class ModelVis;
    /**
     * Properties for an individual static model to be rendered alongside the visualisation
     */
    struct StaticModel {
        StaticModel();
        /**
         * Path to the model to be used
         * @note Must be .obj format
         */
        std::string path;
        /**
         * Optional path to the texture to use for the model
         * @note Leave as empty string if not required
         */
        std::string texture;
        /**
         * 3D size the model should be scaled to
         * @note If x scale is negative, the model will instead be scaled uniformly by -x
         */
        float scale[3];
        /**
         * Translation to be applied to the model
         */
        float location[3];
        /**
         * Rotation to be applied to the model
         * [0-2] Axis of rotation
         * [3] Radians
         */
        float rotation[4];
    };
    explicit ModelConfig(const char *windowTitle);
    ~ModelConfig();
    ModelConfig(const ModelConfig &other);
    ModelConfig &operator=(const ModelConfig &other);
    /**
     * Title of the window which displays the visualisation
     */
    const char* windowTitle;
    /**
     * Initial dimensions of the window which displays the visualisation
     * @note Defaults to (1280, 720)
     */
    unsigned int windowDimensions[2];
    /**
     * The background colour of the visualisation
     * @note This is traditionally Black (0,0,0) or White (1,1,1)
     * @note Defaults to Black (0,0,0)
     */
    float clearColor[3];
    /**
     * Whether the FPS should be displayed in bottom right hand corner
     * @note Defaults to true
     */
    bool fpsVisible;
    /**
     * Colour of the FPS and Step counter overlay text
     * @note Defaults to White (1,1,1)
     */
    float fpsColor[3];
    /**
     * The initial position of the camera
     * @note Defaults to (1.5,1.5,1.5)
     */
    float cameraLocation[3];
    /**
     * The initial point in space which the camera is facing
     * @note Defaults to (0,0,0)
     */
    float cameraTarget[3];
    /**
     * The initial camera roll angle in radians
     */
    float cameraRoll;
    /**
     * The movement speed of the camera in units per millisecond, and the shift key multiplier
     * When shift is pressed the movement speed is multiplied by this value
     * @note Defaults to (0.05,5)
     */
    float cameraSpeed[2];
    /**
     * The near and far clipping planes of the view frustum
     * @note Defaults to (0.05, 5000)
     */
    float nearFarClip[2];
    /**
     * Whether the simulation step counter should be displayed in bottom right hand corner
     * @note Defaults to true
     */
    bool stepVisible;
    /**
     * The number of simulation steps to execute per second
     * A value of 0 is treated as unlimited
     * @note Defaults to 0 (unlimited)
     */
    unsigned int stepsPerSecond = 0;
    /**
     * If true, the simulation will be paused initially, requiring the user to press 'p' to unpause and begin the simulation
     * @note Defaults to false
     */
    bool beginPaused;
    /**
     * Store of static models to be rendered in visualisation
     */
    std::list<std::shared_ptr<StaticModel>> staticModels;
    /**
     * Store of user defined line renderings
     */
    std::list<std::shared_ptr<LineConfig>> lines;
    /**
     * Store of user defined graph renderings
     */
    std::map<std::string, std::shared_ptr<LineConfig>> dynamic_lines;
    /**
     * Store of user defined UI panels
     */
    std::map<std::string, std::shared_ptr<PanelConfig>> panels;
    /**
     * Notify visualisation that it's running under python
     * This mostly just allows the logos to be swapped
     */
    bool isPython;
    /**
     * Use orthographic projection settings instead of perspective
     * This is useful for 2D visualisations
     */
    bool isOrtho = false;
    /**
     * Ortho projection mode uses mousewheel to zoom
     * This var tracks the level of zoom
     */
    float orthoZoom = 1.0f;

 private:
     /**
      * Inline string to char* util
      */
    static void setString(const char ** target, const std::string &src) {
        if (*target)
            free(const_cast<char*>(*target));
        char *t = static_cast<char*>(malloc(src.size() + 1));
        snprintf(t, src.size() + 1, "%s", src.c_str());
        *target = t;
    }
};

}  // namespace visualiser
}  // namespace flamegpu

#endif  // INCLUDE_FLAMEGPU_VISUALISER_CONFIG_MODELCONFIG_H_
