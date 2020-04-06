#include "Visualiser.h"
#include "util/cuda.h"
#include "util/fonts.h"

#include <glm/gtc/matrix_transform.hpp>

#define FOVY 60.0f
#define NEAR_CLIP 0.05f
#define FAR_CLIP 5000.0f
#define DELTA_THETA_PHI 0.01f
#define MOUSE_SPEED 0.001f
#define SHIFT_MULTIPLIER 5.0f

#define MOUSE_SPEED_FPS 0.05f
#define DELTA_MOVE 0.05f
#define DELTA_STRAFE 0.05f
#define DELTA_ASCEND 0.05f
#define DELTA_ROLL 0.01f
#define ONE_SECOND_MS 1000
#define VSYNC 1

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

Visualiser::Visualiser(const ModelConfig& modelcfg)
    : hud(std::make_shared<HUD>(modelcfg.windowDimensions[0], modelcfg.windowDimensions[1]))
    , camera(std::make_shared<NoClipCamera>(glm::vec3(150, 150, 150)))
    // , scene(nullptr)
    , isInitialised(false)
    , continueRender(false)
    , msaaState(true)
    , windowTitle(modelcfg.windowTitle)
    , windowDims(modelcfg.windowDimensions[0], modelcfg.windowDimensions[1])
    , fpsDisplay(nullptr)
    , modelConfig(modelcfg) {
    this->isInitialised = this->init();
    BackBuffer::setClear(true, *reinterpret_cast<const glm::vec3*>(&modelcfg.clearColor[0]));
    if (modelcfg.fpsVisible) {
        fpsDisplay = std::make_shared<Text>("", 10, *reinterpret_cast<const glm::vec3 *>(&modelcfg.fpsColor[0]), fonts::findFont({"Arial"}, fonts::GenericFontFamily::SANS).c_str());
        fpsDisplay->setUseAA(false);
        hud->add(fpsDisplay, HUD::AnchorV::South, HUD::AnchorH::East, glm::ivec2(0), INT_MAX);
        }
}
Visualiser::~Visualiser() {
    this->close();
}
void Visualiser::start() {
    // Only execute if background thread is not active
    if (!isRunning()) {
        if (this->background_thread) {
            // Async window was closed via cross, so thread still exists
            join();  // This kills the thread properly
        }
        // Clear the context from current thread, otherwise we cant move it to background thread
        SDL_GL_MakeCurrent(this->window, NULL);
        SDL_DestroyWindow(this->window);
        // Launch render loop in a new thread
        this->background_thread = new std::thread(&Visualiser::run, this);
    } else {
        printf("Already running! Call quit() to close it first!\n");
    }
}
void Visualiser::join() {
    // Only join if background thread exists, and we are not executing in it
    if (this->background_thread && std::this_thread::get_id() != this->background_thread->get_id()) {
        // Wait for thread to exit
        this->background_thread->join();
        delete this->background_thread;
        this->background_thread = nullptr;
        // Recreate hidden window in current thread, so context is stable
        SDL_GL_MakeCurrent(this->window, NULL);
        SDL_DestroyWindow(this->window);
        this->window = SDL_CreateWindow
        (
            this->windowTitle,
            this->windowedBounds.x,
            this->windowedBounds.y,
            this->windowedBounds.w,
            this->windowedBounds.h,
            SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);  // | SDL_WINDOW_BORDERLESS
        SDL_GL_MakeCurrent(this->window, this->context);
    }
}
void Visualiser::stop() {
    printf("Visualiser::stop()\n");
    this->continueRender = false;
}
void Visualiser::run() {
    if (!this->isInitialised) {
        printf("Visulisation not initialised yet.\n");
    // } else if (!this->scene) {
    //     printf("Scene not yet set.\n");
    } else if (agentStates.size() == 0) {
        printf("No agents set to render.\n");
    } else {
        // Recreate window in current thread (else IO fails)
        if (this->window) {
            SDL_GL_MakeCurrent(this->window, NULL);
            SDL_DestroyWindow(this->window);
        }
        this->window = SDL_CreateWindow
        (
            this->windowTitle,
            this->windowedBounds.x,
            this->windowedBounds.y,
            this->windowedBounds.w,
            this->windowedBounds.h,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);  // | SDL_WINDOW_BORDERLESS
        if (!this->window) {
            printf("Window failed to init.\n");
        } else {
            int err = SDL_GL_MakeCurrent(this->window, this->context);
            if (err != 0) {
                THROW VisAssert("Visualiser::run(): SDL_GL_MakeCurrent failed!\n", SDL_GetError());
            }
            GL_CHECK();
            this->resizeWindow();
            GL_CHECK();
            SDL_StartTextInput();
            this->continueRender = true;
            while (this->continueRender) {
                //  Update the fps in the window title
                this->updateFPS();

                this->render();
            }
            SDL_StopTextInput();
            // Release mouse lock
            if (SDL_GetRelativeMouseMode()) {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
            // Hide window
            SDL_HideWindow(window);
            //  New, might not be required
            SDL_DestroyWindow(this->window);
            this->window = nullptr;
        }
    }
}
void Visualiser::render() {
    // Static fn var for tracking the time to send to scene->update()
    static unsigned int updateTime = 0;
    unsigned int t_updateTime = SDL_GetTicks();
    // If the program runs for over ~49 days, the return value of SDL_GetTicks() will wrap
    unsigned int frameTime = t_updateTime < updateTime ? (t_updateTime + (UINT_MAX - updateTime)) : t_updateTime - updateTime;
    updateTime = t_updateTime;
    SDL_Event e;
    //  Handle continuous key presses (movement)
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    float turboMultiplier = state[SDL_SCANCODE_LSHIFT] ? SHIFT_MULTIPLIER : 1.0f;
    turboMultiplier *= frameTime;
    if (state[SDL_SCANCODE_W]) {
        this->camera->move(DELTA_MOVE*turboMultiplier);
    }
    if (state[SDL_SCANCODE_A]) {
        this->camera->strafe(-DELTA_STRAFE*turboMultiplier);
    }
    if (state[SDL_SCANCODE_S]) {
        this->camera->move(-DELTA_MOVE*turboMultiplier);
    }
    if (state[SDL_SCANCODE_D]) {
        this->camera->strafe(DELTA_STRAFE*turboMultiplier);
    }
    if (state[SDL_SCANCODE_Q]) {
        this->camera->roll(-DELTA_ROLL);
    }
    if (state[SDL_SCANCODE_E]) {
        this->camera->roll(DELTA_ROLL);
    }
    if (state[SDL_SCANCODE_SPACE]) {
        this->camera->ascend(DELTA_ASCEND*turboMultiplier);
    }
    if (state[SDL_SCANCODE_LCTRL]) {
        this->camera->ascend(-DELTA_ASCEND*turboMultiplier);
    }

    //  handle each event on the queue
    while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
        case SDL_QUIT:
            continueRender = false;
            break;
            // case SDL_WINDOWEVENT:
            //     if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            //         resizeWindow();
            //     break;
        case SDL_KEYDOWN: {
            int x = 0;
            int y = 0;
            SDL_GetMouseState(&x, &y);
            this->handleKeypress(e.key.keysym.sym, x, y);
        }
        break;
        // case SDL_MOUSEWHEEL:
        // break;
        case SDL_MOUSEMOTION:
            this->handleMouseMove(e.motion.xrel, e.motion.yrel);
            break;
        case SDL_MOUSEBUTTONDOWN:
            this->toggleMouseMode();
            break;
        }
    }
    //  update
    // this->scene->_update(frameTime);
    //  render
    BackBuffer::useStatic();
    // this->scene->_render();
    renderAgentStates();
    GL_CALL(glViewport(0, 0, windowDims.x, windowDims.y));
    GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    this->hud->render();

    GL_CHECK();

    //  update the screen
    SDL_GL_SwapWindow(window);
}
bool Visualiser::isRunning() const {
    return continueRender;
}
void Visualiser::addAgentState(const std::string &agent_name, const std::string &state_name, const AgentStateConfig &vc) {
    std::pair<std::string, std::string> namepair = { agent_name, state_name };
    GL_CHECK();
    agentStates.emplace(std::make_pair(namepair, RenderInfo(vc)));
    //  Allocate entity
    auto &ent = agentStates.at(namepair).entity;
    ent->setViewMatPtr(camera->getViewMatPtr());
    ent->setProjectionMatPtr(&this->projMat);
    // ent->setLightsBuffer(this->lighting);  //  No lighting yet
}

void Visualiser::renderAgentStates() {
    std::lock_guard<std::mutex> guard(render_buffer_mutex);
    // Resize if necessary
    for (auto &_as : agentStates) {
        auto &as = _as.second;
        // resize buffers
        if (!as.x_var || as.x_var->elementCount < as.requiredSize) {
            //  Decide new buff size
            unsigned int newSize = !as.x_var || as.x_var->elementCount == 0 ? 1024 : as.x_var->elementCount;
            while (newSize < as.requiredSize) {
                newSize = static_cast<unsigned int>(newSize * 1.5f);
            }
            GL_CHECK();
            //  Free old buffs
            if (as.x_var && as.x_var->d_mappedPointer)
                freeGLInteropTextureBuffer(as.x_var);
            if (as.y_var && as.y_var->d_mappedPointer)
                freeGLInteropTextureBuffer(as.y_var);
            if (as.z_var && as.z_var->d_mappedPointer)
                freeGLInteropTextureBuffer(as.z_var);
            GL_CHECK();
            //  Alloc new buffs (this needs to occur in other thread!!!)
            as.x_var = mallocGLInteropTextureBuffer<float>(newSize, 1);
            as.y_var = mallocGLInteropTextureBuffer<float>(newSize, 1);
            as.z_var = mallocGLInteropTextureBuffer<float>(newSize, 1);
            //  Bind texture name to texture unit
            GL_CALL(glActiveTexture(GL_TEXTURE0 + as.tex_unit_offset + 0));
            GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, as.x_var->glTexName));
            GL_CALL(glActiveTexture(GL_TEXTURE0));
            GL_CALL(glActiveTexture(GL_TEXTURE0 + as.tex_unit_offset + 1));
            GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, as.y_var->glTexName));
            GL_CALL(glActiveTexture(GL_TEXTURE0));
            GL_CALL(glActiveTexture(GL_TEXTURE0 + as.tex_unit_offset + 2));
            GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, as.z_var->glTexName));
            GL_CALL(glActiveTexture(GL_TEXTURE0));
            auto shader_vec = as.entity->getShaders();
            GL_CHECK();
            shader_vec->addTexture("x_pos", GL_TEXTURE_BUFFER, as.x_var->glTexName, as.tex_unit_offset + 0);
            shader_vec->addTexture("y_pos", GL_TEXTURE_BUFFER, as.y_var->glTexName, as.tex_unit_offset + 1);
            shader_vec->addTexture("z_pos", GL_TEXTURE_BUFFER, as.z_var->glTexName, as.tex_unit_offset + 2);
            GL_CHECK();
        }
    }
    //  Render agents
    for (auto &as : agentStates) {
        if (as.second.x_var && as.second.x_var->elementCount)  //  Needs to understand struct to see element count
            as.second.entity->renderInstances(as.second.x_var->elementCount);
    }
}
void Visualiser::requestBufferResizes(const std::string &agent_name, const std::string &state_name, const unsigned buffLen) {
    std::pair<std::string, std::string> namepair = { agent_name, state_name };
    auto &as = agentStates.at(namepair);
    as.requiredSize = buffLen;
}
void Visualiser::updateAgentStateBuffer(const std::string &agent_name, const std::string &state_name, const unsigned buffLen, float *d_x, float *d_y, float *d_z) {
    std::pair<std::string, std::string> namepair = { agent_name, state_name };
    auto &as = agentStates.at(namepair);

    //  Copy Data
    if (as.x_var && as.x_var->elementCount >= buffLen) {  //  This may fail for a single frame occasionally
        visassert(_cudaMemcpyDeviceToDevice(as.x_var->d_mappedPointer, d_x, buffLen * sizeof(float)));
        visassert(_cudaMemcpyDeviceToDevice(as.y_var->d_mappedPointer, d_y, buffLen * sizeof(float)));
        visassert(_cudaMemcpyDeviceToDevice(as.z_var->d_mappedPointer, d_z, buffLen * sizeof(float)));
    }
}
//  Used in method above
unsigned int Visualiser::RenderInfo::instanceCounter = 0;

//  Items taken from sdl_exp
bool Visualiser::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    //  Enable MSAA (Must occur before SDL_CreateWindow)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

    // Configure GL buffer settings
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    this->window = SDL_CreateWindow
    (
        this->windowTitle,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        this->windowDims.x,
        this->windowDims.y,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);  // | SDL_WINDOW_BORDERLESS

    if (!this->window) {
        printf("Window failed to init.\n");
    } else {
        SDL_GetWindowPosition(window, &this->windowedBounds.x, &this->windowedBounds.y);
        SDL_GetWindowSize(window, &this->windowedBounds.w, &this->windowedBounds.h);

        //  Get context
        this->context = SDL_GL_CreateContext(window);

        // Enable VSync
        int swapIntervalResult = SDL_GL_SetSwapInterval(VSYNC);
        if (swapIntervalResult == -1) {
            printf("Swap Interval Failed: %s\n", SDL_GetError());
        }

        GLEW_INIT();

        //  Setup gl stuff
        GL_CALL(glEnable(GL_DEPTH_TEST));
        GL_CALL(glCullFace(GL_BACK));
        GL_CALL(glEnable(GL_CULL_FACE));
        GL_CALL(glShadeModel(GL_SMOOTH));
        GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        GL_CALL(glBlendEquation(GL_FUNC_ADD));
        GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        BackBuffer::setClear(true, glm::vec3(0));  // Clear to black
        setMSAA(this->msaaState);

        //  Setup the projection matrix
        this->resizeWindow();
        GL_CHECK();
        return true;
    }
    return false;
}
void Visualiser::setMSAA(bool state) {
    this->msaaState = state;
    if (this->msaaState)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
}
void Visualiser::resizeWindow() {
    //  Use the sdl drawable size
    {
        glm::ivec2 tDims;
        SDL_GL_GetDrawableSize(this->window, &tDims.x, &tDims.y);
        this->windowDims = tDims;
    }
    //  Get the view frustum using GLM. Alternatively glm::perspective could be used.
    this->projMat = glm::perspectiveFov<float>(glm::radians(FOVY), static_cast<float>(this->windowDims.x), static_cast<float>(this->windowDims.y), NEAR_CLIP, FAR_CLIP);
    //  Notify other elements
    this->hud->resizeWindow(this->windowDims);
    //  if (this->scene)
    //     this->scene->_resize(this->windowDims);  //  Not required unless we use multipass framebuffering
    resizeBackBuffer(this->windowDims);
}
void Visualiser::close() {
    continueRender = false;
    if (this->background_thread) {
        this->background_thread->join();
        delete this->background_thread;
    }
    //  This really shouldn't run if we're not the host thread, but we don't manage the render loop thread
    if (this->window != nullptr) {
        SDL_GL_MakeCurrent(this->window, this->context);
        // Delete objects before we delete the GL context!
        deallocateGLObjects();
        SDL_DestroyWindow(this->window);
        this->window = nullptr;
    }
    if (this->context != nullptr) {
        SDL_GL_DeleteContext(this->context);
    }
    SDL_Quit();
}
void Visualiser::handleMouseMove(int x, int y) {
    if (SDL_GetRelativeMouseMode()) {
        this->camera->turn(x * MOUSE_SPEED, y * MOUSE_SPEED);
    }
}
void Visualiser::handleKeypress(SDL_Keycode keycode, int /*x*/, int /*y*/) {
    // Pass key events to the scene and skip handling if false is returned
    // if (scene && !scene->_keypress(keycode, x, y))
    //     return;
    switch (keycode) {
    case SDLK_ESCAPE:
        continueRender = false;
        break;
    case SDLK_F11:
        this->toggleFullScreen();
        break;
    case SDLK_F10:
        this->setMSAA(!this->msaaState);
        break;
    case SDLK_F8:
        if (this->fpsDisplay)
            this->fpsDisplay->setVisible(!this->fpsDisplay->getVisible());
        break;
    case SDLK_F5:
        // if (this->scene)
        //     this->scene->_reload();
        this->hud->reload();
        break;
    default:
        //  Do nothing?
        break;
    }
}
bool Visualiser::isFullscreen() const {
    //  Use window borders as a toggle to detect fullscreen.
    return (SDL_GetWindowFlags(this->window) & SDL_WINDOW_BORDERLESS) == SDL_WINDOW_BORDERLESS;
}
void Visualiser::toggleFullScreen() {
    if (this->isFullscreen()) {
        //  Update the window using the stored windowBounds
        SDL_SetWindowBordered(this->window, SDL_TRUE);
        SDL_SetWindowSize(this->window, this->windowedBounds.w, this->windowedBounds.h);
        SDL_SetWindowPosition(this->window, this->windowedBounds.x, this->windowedBounds.y);
    } else {
        //  Store the windowedBounds for later
        SDL_GetWindowPosition(window, &this->windowedBounds.x, &this->windowedBounds.y);
        SDL_GetWindowSize(window, &this->windowedBounds.w, &this->windowedBounds.h);
        //  Get the window bounds for the current screen
        int displayIndex = SDL_GetWindowDisplayIndex(this->window);
        SDL_Rect displayBounds;
        SDL_GetDisplayBounds(displayIndex, &displayBounds);
        //  Update the window
        SDL_SetWindowBordered(this->window, SDL_FALSE);
        SDL_SetWindowPosition(this->window, displayBounds.x, displayBounds.y);
        SDL_SetWindowSize(this->window, displayBounds.w, displayBounds.h);
    }
    this->resizeWindow();
}
void Visualiser::toggleMouseMode() {
    if (SDL_GetRelativeMouseMode()) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    } else {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }
}
void Visualiser::updateFPS() {
    //  Update the current time
    this->currentTime = SDL_GetTicks();
    //  Update frame counter
    this->frameCount += 1;
    //  If it's been more than a second, do something.
    if (this->currentTime > this->previousTime + ONE_SECOND_MS) {
        //  Calculate average fps.
        double fps = this->frameCount / static_cast<double>(this->currentTime - this->previousTime) * ONE_SECOND_MS;
        // Update the FPS string
        if (this->fpsDisplay)
            this->fpsDisplay->setString("%.3f fps", fps);
        //  reset values;
        this->previousTime = this->currentTime;
        this->frameCount = 0;
    }
}
//  Overrides
unsigned Visualiser::getWindowWidth() const {
    return windowDims.x;
}
unsigned Visualiser::getWindowHeight() const {
    return windowDims.y;
}
glm::uvec2 Visualiser::getWindowDims() const {
    return windowDims;
}
const glm::mat4 * Visualiser::getProjectionMatPtr() const {
    return &projMat;
}
glm::mat4 Visualiser::getProjectionMat() const {
    return projMat;
}
std::shared_ptr<const Camera> Visualiser::getCamera() const {
    return camera;
}
std::weak_ptr<HUD> Visualiser::getHUD() {
    return hud;
}
const char *Visualiser::getWindowTitle() const {
    return windowTitle;
}
void Visualiser::setWindowTitle(const char *_windowTitle) {
    SDL_SetWindowTitle(window, _windowTitle);
    windowTitle = _windowTitle;
}
