#ifndef SRC_VISUALISER_H_
#define SRC_VISUALISER_H_

#include <SDL.h>

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <map>
#undef main  // SDL breaks the regular main entry point, this fixes
#define GLM_FORCE_NO_CTOR_INIT
#include <glm/glm.hpp>

#include "Draw.h"
#include "Entity.h"
#include "HUD.h"
#include "Text.h"
#include "camera/NoClipCamera.h"
#include "config/AgentStateConfig.h"
#include "config/TexBufferConfig.h"
#include "config/ModelConfig.h"
#include "interface/Viewport.h"
#include "Draw.h"
#include "Entity.h"

template <typename T>
struct CUDATextureBuffer;

class LightsBuffer;

/**
 * This is the main class of the visualisation, hosting the window and render loop
 */
class Visualiser : public ViewportExt {
    typedef std::pair<std::string, std::string> NamePair;
    struct NamePairHash {
        size_t operator()(const NamePair &k) const { return std::hash<std::string>()(k.first) ^ (std::hash<std::string>()(k.second) << 1); }
    };
    /**
     * This structs holds the information required for rendering agents for a single agent-state
     */
    struct RenderInfo {
        explicit RenderInfo(const AgentStateConfig &vc,
            const std::map<TexBufferConfig::Function, TexBufferConfig>& core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& tex_buffers);
        RenderInfo(const RenderInfo& vc) = default;
        RenderInfo& operator= (const RenderInfo & vc) = default;
        AgentStateConfig config;
        unsigned int tex_unit_offset;
        unsigned int instanceCount;
        std::map<TexBufferConfig::Function, CUDATextureBuffer<float>*> core_texture_buffers;
        std::multimap<TexBufferConfig::Function, std::pair<CustomTexBufferConfig, CUDATextureBuffer<float>*>> custom_texture_buffers;
        std::shared_ptr<Entity> entity;
        unsigned int requiredSize;  //  Ideally this needs to be threadsafe, but if we make it atomic stuff fails to build
    };

 public:
    explicit Visualiser(const ModelConfig &modelcfg);
    ~Visualiser();
    /**
     * Starts the render loop running
     */
    void start();
    /**
     * Stop the render loop
     */
    void stop();
    /**
     * Block until the render loop has stopped
     * This expects the render loop to exit via the user closing the window (or similar)
     */
    void join();
    /**
     * Returns true if the render loop's thread exists
     */
    bool isRunning() const;
    /**
     * Returns true if the first agent buffer resize has been requested and applied
     */
    bool isReady() const;
    /**
     * Adds the render details for a specific agent state
     * This allows the visualiser to initialise stuff for rendering that agent state
     * @param agent_name Name of the affected agent
     * @param state_name Name of the affected agent state
     * @param vc visualisation config settings for the affected agent state
     * @param core_tex_buffers Meta data to notify required core texture buffers (e.g. location/rotation)
     * @param tex_buffers Meta data to notify required state specific texture buffers (e.g. color)
     */
    void addAgentState(const std::string &agent_name, const std::string &state_name, const AgentStateConfig &vc,
        const std::map<TexBufferConfig::Function, TexBufferConfig>& core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& tex_buffers);
    /**
     * This notifies the render thread that an agent's texture buffers require resizing
     * @param agent_name Name of the affected agent
     * @param state_name Name of the affected agent state
     * @param buffLen New minimum required buffer size
     */
    void requestBufferResizes(const std::string &agent_name, const std::string &state_name, const unsigned int buffLen);
    /**
     * This copies data from the provided device pointers to the texture buffers used for rendering
     * @param agent_name Name of the affected agent
     * @param state_name Name of the affected agent state
     * @param buffLen Number of items to copy
     * @param _core_tex_buffers Device pointers to core texture buffer data
     * @param _tex_buffers Device pointer to custom texture buffer data
     * @note This should only be called if visualisation mutex is held
     * @see getRenderBufferMutex()
     */
    void updateAgentStateBuffer(const std::string &agent_name, const std::string &state_name, const unsigned int buffLen,
        const std::map<TexBufferConfig::Function, TexBufferConfig>& _core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& _tex_buffers);

 private:
    void run();
    /**
     * A single pass of the render loop
     * Also handles keyboard/mouse IO
     */
    void render();
    /**
     * Renders contents of agentStates map
     */
    void renderAgentStates();
    /**
     * Toggles the window between borderless fullscreen and windowed states
     */
    void toggleFullScreen();
    /**
     * @return True if the window is currently full screen
     */
    bool isFullscreen() const;
    /**
     * Toggles whether the mouse is hidden and returned relative to the window
     */
    void toggleMouseMode();
    /**
     * Toggles the status of FPS logged to the HUD
     */
    void toggleFPSStatus();
    /**
     * Toggles whether Multi-Sample Anti-Aliasing should be used or not
     * @param state The desired MSAA state
     * @note Unless blocked by the active Scene the F10 key toggles MSAA at runtime
     */
    void setMSAA(bool state);
    /**
     * Provides key handling for none KEY_DOWN events of utility keys (ESC, F11, F10, F5, etc)
     * @param keycode The keypress detected
     * @param x The horizontal mouse position at the time of the KEY_DOWN event
     * @param y The vertical mouse position at the time of the KEY_DOWN event
     * @note Unsure whether the mouse position is relative to the window
     */
    void handleKeypress(SDL_Keycode keycode, int x, int y);
    /**
     * Moves the camera according to the motion of the mouse (whilst the mouse is attatched to the window via toggleMouseMode())
     * @param x The horizontal distance moved
     * @param y The vertical distance moved
     * @note This is called within the render loop
     */
    void handleMouseMove(int x, int y);
    /**
     * Initialises SDL and creates the window
     * @return Returns true on success
     * @note This method doesn't begin the render loop, use run() for that
     */
    bool init();
    /**
     * Util method which handles deallocating all objects which contains GLbuffers, shaders etc
     */
    void deallocateGLObjects();
    /**
     * Provides destruction of the object, deletes child objects, removes the GL context, closes the window and calls SDL_quit()
     */
    void close();
    /**
     * Simple implementation of an FPS counter
     * @note This is called within the render loop
     */
    void updateFPS();
    /**
     * Updates the viewport and projection matrix
     * This should be called after window resize events, or simply if the viewport needs generating
     */
    void resizeWindow();

    void addController(const SDL_ControllerDeviceEvent sdlEvent);
    void removeController(const SDL_ControllerDeviceEvent sdlEvent);
    void handleControllerButton(const SDL_ControllerButtonEvent sdlEvent);
    void handleControllerAxis(const SDL_ControllerAxisEvent sdlEvent);
    void queryControllerAxis(const unsigned int frameTime);
    void screenshot();
    void screenshot(const bool verbose);

 public:
    /**
     * Returns the window's current width
     * @note This does not account for fullscreen window size
     */
    unsigned int getWindowWidth() const override;
    /**
     * Returns the window's current height
     * @note This does not account for fullscreen window size
     */
    unsigned int getWindowHeight() const override;
    /**
     * Returns the window's current width and height
     * @note This does not account for fullscreen window size
     */
    glm::uvec2 getWindowDims() const override;
    const glm::mat4 *getProjectionMatPtr() const override;
    glm::mat4 getProjectionMat() const override;
    std::shared_ptr<const Camera> getCamera() const override;
    std::weak_ptr<HUD> getHUD() override;
    const char *getWindowTitle() const override;
    void setWindowTitle(const char *windowTitle) override;
    /**
     * Returns the mutex
     * This must be locked before calling updateAgentStateBuffer()
     * @see updateAgentStateBuffer(const std::string &, const std::string &, const unsigned int, float *, float *, float *, float *)
     */
    std::mutex &getRenderBufferMutex() { return render_buffer_mutex; }
    /**
     * Sets the value to be rendered to the HUD step counter (if enabled)
     * @param stepCount The step value to be displayed
     */
    void setStepCount(const unsigned int &stepCount);

 private:
    SDL_Window *window;
    SDL_Rect windowedBounds;
    SDL_GLContext context;
    /**
     * The HUD elements to be rendered
     */
    std::shared_ptr<HUD> hud;
    /**
     * The camera controller for the scene
     */
    std::shared_ptr<NoClipCamera> camera;
    /**
     * The projection matrix for the scene
     * @see resizeWindow()
     */
    glm::mat4 projMat;

    bool isInitialised;
    /**
     * Flag which tells Visualiser to exit the render loop
     */
    std::atomic<bool> continueRender;
    std::atomic<bool> buffersAllocated;
    /**
     * Flag representing whether MSAA is currently enabled
     * MSAA can be toggled at runtime with F10
     * @see setMSAA(bool)
     */
    bool msaaState;
    /**
     * Current title of the visualisation window
     */
    const char *windowTitle;
    /**
     * Current dimensions of the window (does not account for fullscreen size)
     */
    glm::uvec2 windowDims;

    /**
     * Used for tracking and calculating fps
     */
    unsigned int previousTime = 0, currentTime, frameCount = 0;
    /**
     * The object which renders FPS text to the visualisation
     */
    std::shared_ptr<Text> fpsDisplay;
    /**
     * The objects which renders step counter and steps per second text to the visualisation
     */
    std::shared_ptr<Text> stepDisplay, spsDisplay;
    /**
     * When user updates stepCount it is stored here, as we cannot update the OpenGL texture from a thread which doesn't hold the context
     */
    unsigned int previousStepTime = 0, currentStepTime, stepCount = 0, lastStepCount = 0;
    /**
     * Steps equivalent of FPS.
     * Calculated in the wrong thread, so we update it whenever FPS updates
     */
    double stepsPerSecond = 0.0;
    /**
     * Pressing F8 changes whether FPS is displayed in the bottom corner, according to this schema
     * The list is iterated in reverse (2, 1, 0)
     * 0 = Display none
     * 1 = Display step count only
     * 2 = Display step count, sps, fps
     */
    unsigned int fpsStatus = 2;
    /**
     * User defined model configuration options
     */
    ModelConfig modelConfig;
    /**
     * User defined agent states to be rendered and their configuration options
     */
    std::unordered_map<NamePair, RenderInfo, NamePairHash> agentStates;
    /**
     * User defined static models to be rendered
     */
    std::list<std::shared_ptr<Entity>> staticModels;
    /**
     * User defined lines to be rendered
     */
    std::shared_ptr<Draw> lines;
    /**
     * Provides a simple default lighting configuration located at the camera using the old fixed function pipeline methods
     */
    std::shared_ptr<LightsBuffer> lighting;
    /**
     * Counter of how many sepreate line drawings to render,
     * each is named with the std::to_string(i), where i >=0 && i < totalLines
     */
    unsigned int totalLines = 0;
    /**
     * Pressing L during visualisation toggles line rendering on/off
     */
    bool renderLines = true;
    /**
     * Background thread in which visualiser executes
     * (Timestep independent visualiser)
     */
    std::thread *background_thread = nullptr;
    /**
     * Mutex is required to access render buffers for thread safety
     */
    std::mutex render_buffer_mutex;
    /**
     * When this is not set to nullptr, it blocks the simulation from continuing
     */
    std::lock_guard<std::mutex> *pause_guard = nullptr;

    // Controller stuff
    SDL_GameController *gamepad;
    SDL_Joystick *joystick;
    SDL_JoystickID joystickInstance;
    bool gamepadConnected;
};

#endif  // SRC_VISUALISER_H_
