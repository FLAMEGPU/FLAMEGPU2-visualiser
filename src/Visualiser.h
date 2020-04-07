#ifndef SRC_VISUALISER_H_
#define SRC_VISUALISER_H_

#include <atomic>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <utility>
#include <thread>

#include <SDL.h>
#undef main  // SDL breaks the regular main entry point, this fixes
#define GLM_FORCE_NO_CTOR_INIT
#include <glm/glm.hpp>

#include "interface/Viewport.h"
#include "camera/NoClipCamera.h"
#include "HUD.h"
#include "Text.h"

#include "config/ModelConfig.h"
#include "config/AgentStateConfig.h"
#include "Entity.h"

template<typename T>
struct CUDATextureBuffer;

class Visualiser : public ViewportExt {
    typedef std::pair<std::string, std::string> NamePair;
    struct NamePairHash {
        size_t operator()(const NamePair& k) const {
            return std::hash < std::string>()(k.first) ^
                (std::hash < std::string>()(k.second) << 1);
        }
    };
    struct RenderInfo {
        explicit RenderInfo(const AgentStateConfig &vc)
            : config(vc)
            , tex_unit_offset(3 * instanceCounter++)
            , instanceCount(0)
            , x_var(nullptr)
            , y_var(nullptr)
            , z_var(nullptr)
            , entity(std::make_shared<Entity>(
                vc.model_path,
                *reinterpret_cast<const glm::vec3*>(vc.model_scale),
                std::make_shared<Shaders>(
                    "resources/instanced_flat.vert",
                    "resources/material_flat.frag")))
            , requiredSize(0) {
            entity->setMaterial(glm::vec3(0.1f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.7f));
        }
        RenderInfo(const RenderInfo &vc) = default;
        RenderInfo& operator= (const RenderInfo &vc) = default;
        AgentStateConfig config;
        unsigned int tex_unit_offset;
        unsigned int instanceCount;
        CUDATextureBuffer<float> *x_var;
        CUDATextureBuffer<float> *y_var;
        CUDATextureBuffer<float> *z_var;
        std::shared_ptr<Entity> entity;
        unsigned int requiredSize;  //  Ideally this needs to be threadsafe, but if we make it atomic stuff fails to build
        // std::atomic < unsigned int> requiredSize;
        static unsigned int instanceCounter;
    };

 public:
    explicit Visualiser(const ModelConfig& modelcfg);
    ~Visualiser();
    /**
     * Starts the render loop running
     */
    void start();
    void stop();
    void join();
    bool isRunning() const;
    /**
     * Adds the render detials for a specific agent state
     * This allows the visualiser to initialise stuff for rendering that agent state
     */
    void addAgentState(const std::string &agent_name, const std::string &state_name, const AgentStateConfig &vc);
    void requestBufferResizes(const std::string &agent_name, const std::string &state_name, const unsigned int buffLen);
    void updateAgentStateBuffer(const std::string &agent_name, const std::string &state_name, const unsigned int buffLen, float *d_x, float *d_y, float *d_z);

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

 public:
    unsigned getWindowWidth() const override;
    unsigned getWindowHeight() const override;
    glm::uvec2 getWindowDims() const override;
    const glm::mat4 * getProjectionMatPtr() const override;
    glm::mat4 getProjectionMat() const override;
    std::shared_ptr<const Camera> getCamera() const override;
    std::weak_ptr<HUD> getHUD() override;
    const char * getWindowTitle() const override;
    void setWindowTitle(const char *windowTitle) override;
    std::mutex &getRenderBufferMutex() { return render_buffer_mutex; }

 private:
    SDL_Window* window;
    SDL_Rect windowedBounds;
    SDL_GLContext context;

    std::shared_ptr<HUD> hud;
    std::shared_ptr<NoClipCamera> camera;
    // std::shared_ptr<Scene> scene;
    glm::mat4 projMat;

    bool isInitialised;
    /**
     * Flag which tells Visualiser to exit the render loop
     */
    std::atomic < bool> continueRender;

    bool msaaState;

    const char* windowTitle;
    glm::uvec2 windowDims;

    // FPS tracking stuff
    unsigned int previousTime = 0;
    unsigned int currentTime;
    unsigned int frameCount = 0;
    std::shared_ptr<Text> fpsDisplay;

    ModelConfig modelConfig;
    std::unordered_map<NamePair, RenderInfo, NamePairHash> agentStates;

    /**
     * Background thread in which visualiser executes
     * (Timestep independent visualiser)
     */
    std::thread *background_thread = nullptr;
    std::mutex render_buffer_mutex;
    /**
     * When this is not set to nullptr, it blocks the simulation from continuing
     */
    std::lock_guard<std::mutex> *pause_guard = nullptr;
};

#endif  // SRC_VISUALISER_H_
