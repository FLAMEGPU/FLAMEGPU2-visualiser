#include "Visualiser.h"

#include <iomanip>
#include <sstream>

#include <glm/gtc/matrix_transform.hpp>

#include "util/cuda.h"
#include "util/fonts.h"
#include "shader/DirectionFunction.h"
#include "shader/ScaleFunction.h"
#include "shader/lights/LightsBuffer.h"
#include "ui/Text.h"
#include "ui/SplashScreen.h"


#define FOVY 60.0f
#define DELTA_THETA_PHI 0.01f
#define MOUSE_SPEED 0.001f

#define MOUSE_SPEED_FPS 0.05f
#define DELTA_ROLL 0.01f
#define ONE_SECOND_MS 1000
#define VSYNC 1

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

#define MOVEMENT_MULTIPLIER 1.f
#define AXIS_LEFT_DEAD_ZONE 0.2f
#define AXIS_RIGHT_DEAD_ZONE 0.15f
#define AXIS_TRIGGER_DEAD_ZONE 0.25f
#define AXIS_DELTA_MOVE (-0.075f * MOVEMENT_MULTIPLIER)
#define AXIS_DELTA_STRAFE (0.05f * MOVEMENT_MULTIPLIER)
#define AXIS_DELTA_ASCEND (0.05f * MOVEMENT_MULTIPLIER)
#define AXIS_DELTA_ROLL 0.05f
#define AXIS_DELTA_THETA 0.015f
#define AXIS_DELTA_PHI 0.015f
#define AXIS_TURN_THRESHOLD 0.03f

Visualiser::RenderInfo::RenderInfo(const AgentStateConfig& vc,
    const std::map<TexBufferConfig::Function, TexBufferConfig>& _core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>&_custom_tex_buffers)
    : config(vc)
    , tex_unit_offset(0)
    , instanceCount(0)
    , entity(nullptr)
    , requiredSize(0)
    , dataSize(0) {
        // Copy texture buffers to dedicated structures
        for (auto &c : _core_tex_buffers) {
            core_texture_buffers.emplace(c.first, nullptr);
        }
        for (auto& c : _custom_tex_buffers) {
            custom_texture_buffers.emplace(c.first, std::make_pair(c.second, nullptr));
        }
        // Select the corresponding shader
        DirectionFunction df(_core_tex_buffers);
        ScaleFunction sf(_core_tex_buffers);
        if (!vc.color_shader_src.empty()) {
            // Entity has a color and direction override
            entity = std::make_shared<Entity>(
                vc.model_path,
                *reinterpret_cast<const glm::vec3*>(vc.model_scale),
                std::make_shared<Shaders>(
                    "resources/instanced_default_Tcolor_Tdir_Tscale.vert",
                    "resources/material_flat_Tcolor.frag",
                    "",
                    sf.getSrc() + df.getSrc() + vc.color_shader_src));
        } else if (vc.model_texture) {
            // Entity has texture
            entity = std::make_shared<Entity>(
                vc.model_path,
                *reinterpret_cast<const glm::vec3*>(vc.model_scale),
                std::make_shared<Shaders>(
                    "resources/instanced_default_Tdir_Tscale.vert",
                    "resources/material_phong.frag",
                    "",
                    sf.getSrc() + df.getSrc()),
                Texture2D::load(vc.model_texture));
        } else {
            // Entity does not have a texture
            entity = std::make_shared<Entity>(
                vc.model_path,
                *reinterpret_cast<const glm::vec3*>(vc.model_scale),
                std::make_shared<Shaders>(
                    "resources/instanced_default_Tdir_Tscale.vert",
                    "resources/material_flat.frag",
                    "",
                    sf.getSrc() + df.getSrc()));
            entity->setMaterial(glm::vec3(0.1f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.7f));
        }
}

Visualiser::Visualiser(const ModelConfig& modelcfg)
    : hud(std::make_shared<HUD>(modelcfg.windowDimensions[0], modelcfg.windowDimensions[1]))
    , camera(std::make_shared<NoClipCamera>(*reinterpret_cast<const glm::vec3*>(&modelcfg.cameraLocation[0]), *reinterpret_cast<const glm::vec3*>(&modelcfg.cameraTarget[0])))
    , isInitialised(false)
    , continueRender(false)
    , buffersAllocated(false)
    , msaaState(true)
    , windowTitle(modelcfg.windowTitle)
    , windowDims(modelcfg.windowDimensions[0], modelcfg.windowDimensions[1])
    , splashScreen(nullptr)
    , fpsDisplay(nullptr)
    , stepDisplay(nullptr)
    , spsDisplay(nullptr)
    , debugMenu(nullptr)
    , modelConfig(modelcfg)
    , lighting(nullptr)
    , gamepad(nullptr)
    , joystick(nullptr)
    , joystickInstance(0)
    , gamepadConnected(false) {
    this->isInitialised = this->init();
    // Init splash screen
    splashScreen = std::make_shared<SplashScreen>(*reinterpret_cast<const glm::vec3*>(&modelcfg.fpsColor[0]), "Loading...", modelcfg.isPython);
    hud->add(splashScreen, HUD::AnchorV::Center, HUD::AnchorH::Center, glm::ivec2(0, 0), INT_MAX);
    lighting = std::make_shared<LightsBuffer>(camera->getViewMatPtr());
    // Apply user specified stuff
    BackBuffer::setClear(true, *reinterpret_cast<const glm::vec3*>(&modelcfg.clearColor[0]));
    if (modelcfg.fpsVisible) {
        fpsDisplay = std::make_shared<Text>("", 10, *reinterpret_cast<const glm::vec3 *>(&modelcfg.fpsColor[0]), fonts::findFont({"Arial"}, fonts::GenericFontFamily::SANS).c_str());
        fpsDisplay->setUseAA(false);
        hud->add(fpsDisplay, HUD::AnchorV::South, HUD::AnchorH::East, glm::ivec2(0, modelcfg.stepVisible ? 20 : 0), INT_MAX - 1);
    }
    if (modelcfg.stepVisible) {
        spsDisplay = std::make_shared<Text>("", 10, *reinterpret_cast<const glm::vec3*>(&modelcfg.fpsColor[0]), fonts::findFont({ "Arial" }, fonts::GenericFontFamily::SANS).c_str());
        spsDisplay->setUseAA(false);
        hud->add(spsDisplay, HUD::AnchorV::South, HUD::AnchorH::East, glm::ivec2(0, 10), INT_MAX - 1);
        stepDisplay = std::make_shared<Text>("", 10, *reinterpret_cast<const glm::vec3 *>(&modelcfg.fpsColor[0]), fonts::findFont({"Arial"}, fonts::GenericFontFamily::SANS).c_str());
        stepDisplay->setUseAA(false);
        hud->add(stepDisplay, HUD::AnchorV::South, HUD::AnchorH::East, glm::ivec2(0, 0), INT_MAX - 1);
    }
    {  // Debug menu
        debugMenu = std::make_shared<Text>("", 16, glm::vec3(1.0f), fonts::findFont({ "Consolas", "Arial" }, fonts::GenericFontFamily::SANS).c_str());
        debugMenu->setBackgroundColor(glm::vec4(*reinterpret_cast<const glm::vec3*>(&modelcfg.clearColor[0]), 0.65f));
        debugMenu->setVisible(false);
        hud->add(debugMenu, HUD::AnchorV::North, HUD::AnchorH::West, glm::ivec2(0, 0), INT_MAX);
    }
    lines = std::make_shared<Draw>();
    lines->setViewMatPtr(camera->getViewMatPtr());
    lines->setProjectionMatPtr(&this->projMat);
    // Process static models
    for (auto &sm : modelcfg.staticModels) {
        std::shared_ptr<Entity> entity;
        if (sm->texture.empty()) {
            // Entity does not have a texture
            entity = std::make_shared<Entity>(
                sm->path.c_str(),
                *reinterpret_cast<const glm::vec3*>(sm->scale),
                std::make_shared<Shaders>(
                    "resources/default.vert",
                    "resources/material_flat.frag"));
            entity->setMaterial(glm::vec3(0.1f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.7f));
        } else {
            // Entity has texture
            entity = std::make_shared<Entity>(
                sm->path.c_str(),
                *reinterpret_cast<const glm::vec3*>(&sm->scale),
                std::make_shared<Shaders>(
                    "resources/default.vert",
                    "resources/material_phong.frag"),
                Texture2D::load(sm->texture));
        }
        if (entity) {
            entity->setLocation(*reinterpret_cast<const glm::vec3*>(sm->location));
            entity->setRotation(*reinterpret_cast<const glm::vec4*>(sm->rotation));
            entity->setViewMatPtr(camera->getViewMatPtr());
            entity->setProjectionMatPtr(&this->projMat);
            entity->setLightsBuffer(this->lighting);

            staticModels.push_back(entity);
        }
    }
    // Process lines
    for (auto &line : modelcfg.lines) {
        // Check it's valid
        if (line->lineType == LineConfig::Type::Polyline) {
            if (line->vertices.size() < 6 || line->vertices.size() % 3 != 0 || line->colors.size() % 4 != 0 || (line->colors.size() / 4) * 3 != line->vertices.size()) {
                THROW SketchError("Polyline sketch contains invalid number of vertices (%d/3) or colours (%d/4).\n", line->vertices.size(), line->colors.size());
            }
        } else if (line->lineType == LineConfig::Type::Lines) {
            if (line->vertices.size() < 6 || line->vertices.size() % 6 != 0 || line->colors.size() % 4 != 0 || (line->colors.size() / 4) * 3 != line->vertices.size()) {
                THROW SketchError("Lines sketch contains invalid number of vertices (%d/3) or colours (%d/4).\n", line->vertices.size(), line->colors.size());
            }
        }
        // Convert to Draw
        lines->begin(line->lineType == LineConfig::Type::Polyline ? Draw::Type::Polyline : Draw::Type::Lines, std::to_string(totalLines++));
        for (size_t i = 0; i < line->vertices.size() / 3; ++i) {
            lines->color(*reinterpret_cast<const glm::vec4*>(&line->colors[i * 4]));
            lines->vertex(*reinterpret_cast<const glm::vec3*>(&line->vertices[i * 3]));
        }
        lines->save();
    }
    // Default lighting, single point light attached to camera
    // Maybe in future let user specify lights instead of this
    {
        PointLight _p = lighting->addPointLight();
        _p.Ambient(glm::vec3(0.2f));
        _p.Diffuse(glm::vec3(1.0f));
        _p.Specular(glm::vec3(0.02f));
        _p.ConstantAttenuation(1.0f);
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
        this->continueRender = true;
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
        this->window = SDL_CreateWindow(
            this->windowTitle,
            this->windowedBounds.x,
            this->windowedBounds.y,
            this->windowedBounds.w,
            this->windowedBounds.h,
            SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);  // | SDL_WINDOW_BORDERLESS
        setWindowIcon();
        SDL_GL_MakeCurrent(this->window, this->context);
    }
}
void Visualiser::stop() {
    printf("Visualiser::stop()\n");
    this->continueRender = false;
}
void Visualiser::run() {
    if (!this->isInitialised) {
        printf("Visualisation not initialised yet.\n");
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
        this->window = SDL_CreateWindow(
            this->windowTitle,
            this->windowedBounds.x,
            this->windowedBounds.y,
            this->windowedBounds.w,
            this->windowedBounds.h,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);  // | SDL_WINDOW_BORDERLESS
        if (!this->window) {
            printf("Window failed to init.\n");
        } else {
            setWindowIcon();
            int err = SDL_GL_MakeCurrent(this->window, this->context);
            if (err != 0) {
                THROW VisAssert("Visualiser::run(): SDL_GL_MakeCurrent failed!\n", SDL_GetError());
            }
            GL_CHECK();
            this->resizeWindow();
            GL_CHECK();
            SDL_StartTextInput();
            while (this->continueRender) {
                //  Update the fps in the window title
                this->updateFPS();
                if (this->stepDisplay) {
                    this->spsDisplay->setString("%.3f sps", stepsPerSecond);
                    this->stepDisplay->setString("%sStep %u", (pause_guard? "(Paused) " : ""), stepCount);
                }
                this->render();
            }
            SDL_StopTextInput();
            // Release mouse lock
            if (SDL_GetRelativeMouseMode()) {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
            // Un-pause the simulation if required.
            if (this->pause_guard) {
                delete this->pause_guard;
                this->pause_guard = nullptr;
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
    const unsigned int t_updateTime = SDL_GetTicks();
    // If the program runs for over ~49 days, the return value of SDL_GetTicks() will wrap
    const unsigned int frameTime = t_updateTime < updateTime ? (t_updateTime + (UINT_MAX - updateTime)) : t_updateTime - updateTime;
    updateTime = t_updateTime;
    SDL_Event e;
    //  Handle continuous key presses (movement)
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    float speed = modelConfig.cameraSpeed[0];
    if (state[SDL_SCANCODE_LSHIFT]) {
        speed *= modelConfig.cameraSpeed[1];
    }
    if (state[SDL_SCANCODE_LCTRL]) {
        speed /= modelConfig.cameraSpeed[1];
    }
    const float distance = speed * static_cast<float>(frameTime);
    if (state[SDL_SCANCODE_W]) {
        this->camera->move(distance);
    }
    if (state[SDL_SCANCODE_A]) {
        this->camera->strafe(-distance);
    }
    if (state[SDL_SCANCODE_S]) {
        this->camera->move(-distance);
    }
    if (state[SDL_SCANCODE_D]) {
        this->camera->strafe(distance);
    }
    if (state[SDL_SCANCODE_Q]) {
        this->camera->roll(-DELTA_ROLL);
    }
    if (state[SDL_SCANCODE_E]) {
        this->camera->roll(DELTA_ROLL);
    }
    if (state[SDL_SCANCODE_SPACE]) {
        this->camera->ascend(distance);
    }
    if (state[SDL_SCANCODE_C]) {  // Ctrl now moves slower
        this->camera->ascend(-distance);
    }
    // After movement update default light position
    this->lighting->getPointLight(0).Position(this->camera->getEye());
    //  handle each event on the queue
    while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
        case SDL_QUIT:
            continueRender = false;
            break;
        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                resizeWindow();
            break;
        case SDL_KEYDOWN:
            {
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
        case SDL_CONTROLLERDEVICEADDED:
            this->addController(e.cdevice);
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            this->removeController(e.cdevice);
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            this->handleControllerButton(e.cbutton);
            break;
        case SDL_CONTROLLERBUTTONUP:
            this->handleControllerButton(e.cbutton);
            break;
        // case SDL_CONTROLLERAXISMOTION:
            // Couldn't get this event to behave nicely in the past for some reason.
            // break;
        }
    }
    // Doesn't use the event class for some historical reason I (pth) don't remember.
    this->queryControllerAxis(frameTime);
    // Update lighting
    lighting->update();
    updateDebugMenu();
    //  render
    BackBuffer::useStatic();
    for (auto &sm : staticModels)
        sm->render();
    renderAgentStates();
    // Close splash screen if we are ready (renderAgentStates sets this)
    if (closeSplashScreen && splashScreen) {
        splashScreen->setVisible(false);  // redundant
        hud->remove(splashScreen);
        splashScreen.reset();
    }
    // Render lines last, as they may contain alpha
    if (renderLines) {
        GL_CALL(glEnable(GL_BLEND));
        GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        for (unsigned int i = 0; i < totalLines; ++i)
            lines->render(std::to_string(i));
        GL_CALL(glDisable(GL_BLEND));
    }
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
bool Visualiser::isReady() const {
    return buffersAllocated;
}
void Visualiser::addAgentState(const std::string &agent_name, const std::string &state_name, const AgentStateConfig &vc,
    const std::map<TexBufferConfig::Function, TexBufferConfig>& core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& tex_buffers) {
    std::pair<std::string, std::string> namepair = { agent_name, state_name };
    GL_CHECK();
    agentStates.emplace(std::make_pair(namepair, RenderInfo(vc, core_tex_buffers, tex_buffers)));
    //  Allocate entity
    auto &ent = agentStates.at(namepair).entity;
    ent->setViewMatPtr(camera->getViewMatPtr());
    ent->setProjectionMatPtr(&this->projMat);
    ent->setLightsBuffer(this->lighting);
    // Check if there are multiple agent states
    for (auto &as : agentStates) {
        if (as.first.second != state_name) {
            debugMenu_showStateNames = true;
            return;
        }
    }
}

void Visualiser::renderAgentStates() {
    static unsigned int texture_unit_counter = 1;
    std::lock_guard<std::mutex> *guard = nullptr;
    if (!pause_guard)
        guard = new std::lock_guard<std::mutex>(render_buffer_mutex);
    bool hasResized = false;
    // Resize if necessary
    for (auto &_as : agentStates) {
        auto &as = _as.second;
        if (as.core_texture_buffers.empty())
            return;
        // resize buffers
        const auto &first_buff = as.core_texture_buffers.begin()->second;
        const unsigned int allocated_size = first_buff ? first_buff->elementCount : 0;
        if (allocated_size < as.requiredSize) {
            // If we haven't been allocated texture units yet, get them now
            if (!as.tex_unit_offset) {
                as.tex_unit_offset = texture_unit_counter;
                texture_unit_counter += static_cast<unsigned int>(as.core_texture_buffers.size() + as.custom_texture_buffers.size());
            }
            //  Decide new buff size
            unsigned int newSize = allocated_size < 1024 ? 1024 : allocated_size;
            while (newSize < as.requiredSize) {
                newSize = static_cast<unsigned int>(newSize * 1.5f);
            }
            GL_CHECK();
            auto shader_vec = as.entity->getShaders();
            unsigned int tui = 0;
            for (auto &_tb : as.core_texture_buffers) {
                auto &tb = _tb.second;
                const std::string samplerName = TexBufferConfig::SamplerName(_tb.first);
                // Remove old buff from shader
                shader_vec->removeTextureUniform(samplerName.c_str());
                CUDATextureBuffer<float> *old_tb = tb;
                // Alloc new buffs (this needs to occur in render thread!)
                tb = mallocGLInteropTextureBuffer<float>(newSize, 1);
                // Copy any old data to the buffer
                if (old_tb && tb && old_tb->d_mappedPointer && tb->d_mappedPointer && as.dataSize)
                    _cudaMemcpyDeviceToDevice(tb->d_mappedPointer, old_tb->d_mappedPointer, as.dataSize * sizeof(float));
                // Bind texture name to texture unit
                GL_CALL(glActiveTexture(GL_TEXTURE0 + as.tex_unit_offset + tui));
                GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, tb->glTexName));
                GL_CALL(glActiveTexture(GL_TEXTURE0));
                shader_vec->addTexture(samplerName.c_str(), GL_TEXTURE_BUFFER, tb->glTexName, as.tex_unit_offset + tui);
                // Free old buff
                if (old_tb && old_tb->d_mappedPointer)
                    freeGLInteropTextureBuffer(old_tb);
                ++tui;
                GL_CHECK();
            }
            for (auto& _tb : as.custom_texture_buffers) {
                auto& tb = _tb.second.second;
                // Remove old buff from shader
                shader_vec->removeTextureUniform(_tb.second.first.nameInShader.c_str());
                CUDATextureBuffer<float>* old_tb = tb;
                // Alloc new buffs (this needs to occur in other thread!!!)
                tb = mallocGLInteropTextureBuffer<float>(newSize, 1);
                // Copy any old data to the buffer
                if (old_tb && tb && old_tb->d_mappedPointer && tb->d_mappedPointer && as.dataSize)
                    _cudaMemcpyDeviceToDevice(tb->d_mappedPointer, old_tb->d_mappedPointer, as.dataSize * sizeof(float));
                // Bind texture name to texture unit
                GL_CALL(glActiveTexture(GL_TEXTURE0 + as.tex_unit_offset + tui));
                GL_CALL(glBindTexture(GL_TEXTURE_BUFFER, tb->glTexName));
                GL_CALL(glActiveTexture(GL_TEXTURE0));
                shader_vec->addTexture(_tb.second.first.nameInShader.c_str(), GL_TEXTURE_BUFFER, tb->glTexName, as.tex_unit_offset + tui);
                // Free old buff
                if (old_tb && old_tb->d_mappedPointer)
                    freeGLInteropTextureBuffer(old_tb);
                ++tui;
                GL_CHECK();
            }
            hasResized = true;
        }
    }
    if (hasResized)
        buffersAllocated = true;

    // Check that all buffers with a requested size, actually have data before we render
    // This prevents an initial frame where only some agents are rendered.
    if (!closeSplashScreen && buffersAllocated) {
        closeSplashScreen = true;
        for (auto& as : agentStates) {
            if (as.second.requiredSize && !as.second.dataSize) {
                closeSplashScreen = false;
                break;
            }
        }
        // If desired, enable pause once splash screen has closed
        if (closeSplashScreen) {
            if (!this->pause_guard && modelConfig.beginPaused) {
                this->pause_guard = guard;
                guard = nullptr;
            }
        }
    }

    //  Render agents
    if (closeSplashScreen) {
        for (auto &as : agentStates) {
            if (!as.second.core_texture_buffers.empty() && as.second.dataSize)  // Extra check to make sure buffer has been allocated successfully
                as.second.entity->renderInstances(as.second.dataSize);
        }
    }
    if (guard)
        delete guard;
}
void Visualiser::requestBufferResizes(const std::string &agent_name, const std::string &state_name, const unsigned buffLen, bool force) {
    std::pair<std::string, std::string> namepair = { agent_name, state_name };
    auto &as = agentStates.at(namepair);
    // Mark sim as not ready if forced
    if (as.requiredSize != buffLen && force)
        buffersAllocated = false;
    // Update required size
    as.requiredSize = buffLen;
}
void Visualiser::updateAgentStateBuffer(const std::string &agent_name, const std::string &state_name, const unsigned buffLen,
    const std::map<TexBufferConfig::Function, TexBufferConfig>& ext_core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& ext_tex_buffers) {
    std::pair<std::string, std::string> namepair = { agent_name, state_name };
    auto &as = agentStates.at(namepair);
    if (as.core_texture_buffers.empty() || buffLen == 0)
        return;
    //  Copy Data
    const auto& first_buff = as.core_texture_buffers.begin()->second;
    const unsigned int buff_size = first_buff ? first_buff->elementCount : 0;
    if (buff_size) {
        // If buffer has to resize, we may not copy data for new agents
        as.dataSize = buffLen < buff_size ? buffLen : buff_size;
        for (const auto &_ext_tb : ext_core_tex_buffers) {
            auto &ext_tb = _ext_tb.second;
            auto &int_tb = as.core_texture_buffers.at(_ext_tb.first);
            visassert(_cudaMemcpyDeviceToDevice(int_tb->d_mappedPointer, ext_tb.t_d_ptr, as.dataSize * sizeof(float)));
        }
        for (const auto& _ext_tb : ext_tex_buffers) {
            auto& ext_tb = _ext_tb.second;
            for (auto int_tb = as.custom_texture_buffers.find(_ext_tb.first); int_tb != as.custom_texture_buffers.end(); ++int_tb) {
                if (ext_tb.nameInShader == int_tb->second.first.nameInShader) {
                    visassert(_cudaMemcpyDeviceToDevice(int_tb->second.second->d_mappedPointer, ext_tb.t_d_ptr, as.dataSize * sizeof(float)));
                }
            }
        }
    }
}

//  Items taken from sdl_exp
bool Visualiser::init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
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

    this->window = SDL_CreateWindow(
        this->windowTitle,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        this->windowDims.x,
        this->windowDims.y,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);  // | SDL_WINDOW_BORDERLESS

    if (!this->window) {
        printf("Window failed to init.\n");
    } else {
        setWindowIcon();
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
    this->projMat = glm::perspectiveFov<float>(
        glm::radians(FOVY),
        static_cast<float>(this->windowDims.x),
        static_cast<float>(this->windowDims.y),
        modelConfig.nearFarClip[0],
        modelConfig.nearFarClip[1]);
    //  Notify other elements
    this->hud->resizeWindow(this->windowDims);
    //  if (this->scene)
    //     this->scene->_resize(this->windowDims);  //  Not required unless we use multipass framebuffering
    resizeBackBuffer(this->windowDims);
}
void Visualiser::deallocateGLObjects() {
    splashScreen.reset();
    lighting.reset();
    fpsDisplay.reset();
    stepDisplay.reset();
    spsDisplay.reset();
    debugMenu.reset();
    this->hud->clear();
    // Don't clear the map, as update buffer methods might still be called
    for (auto &as : agentStates) {
        as.second.entity.reset();
    }
    this->lines.reset();
}

void Visualiser::close() {
    continueRender = false;
    if (this->pause_guard) {
        delete this->pause_guard;
        this->pause_guard = nullptr;
    }
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
void Visualiser::toggleFPSStatus() {
    // Update fpsStatus
    fpsStatus = fpsStatus == 0 ? 2u : fpsStatus - 1;
    // If steps is not visible, skip state 1
    if (!this->stepDisplay && fpsStatus == 1)
        fpsStatus = 0;
    // Set the appropriate display state
    switch (fpsStatus) {
    case 0:  // Hide all
        if (this->stepDisplay)
            this->stepDisplay->setVisible(false);
    case 1:  // Hide fps/sps
        if (this->fpsDisplay)
            this->fpsDisplay->setVisible(false);
        if (this->spsDisplay)
            this->spsDisplay->setVisible(false);
        break;
    default:  // Show all
        if (this->stepDisplay)
            this->stepDisplay->setVisible(true);
        if (this->fpsDisplay)
            this->fpsDisplay->setVisible(true);
        if (this->spsDisplay)
            this->spsDisplay->setVisible(true);
        break;
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
    case SDLK_F12:  // If using VS Debugger F12 forces break, Ctrl + F12 however works for this without breaking
        this->screenshot(true);
        break;
    case SDLK_F11:
        this->toggleFullScreen();
        break;
    case SDLK_F10:
        this->setMSAA(!this->msaaState);
        break;
    case SDLK_F8:
        this->toggleFPSStatus();
        break;
    case SDLK_F5:
        // Reload all shaders
        if (this->lines)
            this->lines->reload();
        for (auto& as : this->agentStates)
            as.second.entity->reload();
        for (auto& sm : this->staticModels)
            sm->reload();
        this->hud->reload();
        break;
    case SDLK_F1:
        if (this->debugMenu)
            this->debugMenu->setVisible(!this->debugMenu->getVisible());
        break;
    case SDLK_p:
        if (this->pause_guard) {
            delete pause_guard;
            pause_guard = nullptr;
        } else {
            pause_guard = new std::lock_guard<std::mutex>(render_buffer_mutex);
            stepsPerSecond = 0.0;
        }
        break;
    case SDLK_l:
        renderLines = !renderLines;
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
    // this->resizeWindow(); will be triggered by SDL_WINDOWEVENT_SIZE_CHANGED
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
void Visualiser::setStepCount(const unsigned int &_stepCount) {
    // This boring value is used to display the number of steps
    stepCount = _stepCount;
    // The rest is to calcualted steps/second, basically the same as FPS
    //  Update the current time
    this->currentStepTime = SDL_GetTicks();
    //  If it's been more than a second, do something.
    if (this->currentStepTime > this->previousStepTime + ONE_SECOND_MS) {
        //  Calculate average fps.
        this->stepsPerSecond = (_stepCount - this->lastStepCount) / static_cast<double>(this->currentStepTime - this->previousStepTime) * ONE_SECOND_MS;
        // Don't update string here, wrong thread to do OpenGL
        //  reset values;
        this->previousStepTime = this->currentStepTime;
        this->lastStepCount = _stepCount;
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

// Game controller methods
void Visualiser::addController(const SDL_ControllerDeviceEvent sdlEvent) {
    int device = sdlEvent.which;
    // If it is the 0th gamepad, proceed.
    if (device == 0) {
        this->gamepad = SDL_GameControllerOpen(device);
        this->joystick = SDL_GameControllerGetJoystick(gamepad);
        this->joystickInstance = SDL_JoystickInstanceID(this->joystick);
        this->gamepadConnected = true;
    }
}
void Visualiser::removeController(const SDL_ControllerDeviceEvent sdlEvent) {
    if (this->gamepadConnected) {
        this->gamepadConnected = false;
        SDL_GameControllerClose(this->gamepad);
        this->gamepad = 0;
    }
}
// @todo - handle continuous button states
void Visualiser::handleControllerButton(const SDL_ControllerButtonEvent sdlEvent) {
    // If it is the correct controller
    if (sdlEvent.which == this->joystickInstance) {
        // If it is being pressed down
        if (sdlEvent.type == SDL_CONTROLLERBUTTONDOWN) {
            // Print the button being pressed.
            switch (sdlEvent.button) {
            case SDL_CONTROLLER_BUTTON_A:
                break;
            case SDL_CONTROLLER_BUTTON_B:
                break;
            case SDL_CONTROLLER_BUTTON_X:
                // this->camera->resetLocation();
                break;
            case SDL_CONTROLLER_BUTTON_Y:
                // this->camera->storeLocation();
                break;
            case SDL_CONTROLLER_BUTTON_BACK:
                this->toggleFPSStatus();
                break;
            case SDL_CONTROLLER_BUTTON_GUIDE:
                break;
            case SDL_CONTROLLER_BUTTON_START:
                if (this->pause_guard) {
                    delete pause_guard;
                    pause_guard = nullptr;
                } else {
                    pause_guard = new std::lock_guard<std::mutex>(render_buffer_mutex);
                    stepsPerSecond = 0.0f;
                }
                break;
            case SDL_CONTROLLER_BUTTON_LEFTSTICK:
                break;
            case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
                break;
            case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                break;
            case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                this->toggleFullScreen();
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                this->screenshot(true);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                renderLines = !renderLines;
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                this->hud->reload();
                break;
            case SDL_CONTROLLER_BUTTON_MAX:
                break;
            default:
                break;
            }
        }
        // else if (sdlEvent.type == SDL_CONTROLLERBUTTONUP){
        // }
    }
}
// If there is a controllwer attached update all relevant values
void Visualiser::queryControllerAxis(const unsigned int frameTime) {
    if (this->gamepadConnected) {
        // Load in all values
        float leftX = SDL_GameControllerGetAxis(this->gamepad, SDL_CONTROLLER_AXIS_LEFTX) / static_cast<float>(SHRT_MAX);
        float leftY = SDL_GameControllerGetAxis(this->gamepad, SDL_CONTROLLER_AXIS_LEFTY) / static_cast<float>(SHRT_MAX);
        float rightX = SDL_GameControllerGetAxis(this->gamepad, SDL_CONTROLLER_AXIS_RIGHTX) / static_cast<float>(SHRT_MAX);
        float rightY = SDL_GameControllerGetAxis(this->gamepad, SDL_CONTROLLER_AXIS_RIGHTY) / static_cast<float>(SHRT_MAX);
        float triggerL = SDL_GameControllerGetAxis(this->gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / static_cast<float>(SHRT_MAX);
        float triggerR = SDL_GameControllerGetAxis(this->gamepad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / static_cast<float>(SHRT_MAX);
        float buttonA = SDL_GameControllerGetButton(this->gamepad, SDL_CONTROLLER_BUTTON_A);
        float buttonB = SDL_GameControllerGetButton(this->gamepad, SDL_CONTROLLER_BUTTON_B);
        float leftShoulder = SDL_GameControllerGetButton(this->gamepad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        float rightShoulder = SDL_GameControllerGetButton(this->gamepad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);

        float speed = modelConfig.cameraSpeed[0];
        if (triggerL > AXIS_TRIGGER_DEAD_ZONE) {
            speed /= modelConfig.cameraSpeed[1];
        }
        if (triggerR > AXIS_TRIGGER_DEAD_ZONE) {
            speed *= modelConfig.cameraSpeed[1];
        }
        const float distance = speed * static_cast<float>(frameTime);

        // Strafe on left X axis
        if (abs(leftX) > AXIS_LEFT_DEAD_ZONE) {
            this->camera->strafe(leftX * distance);
        }
        // Move on left Y axis
        if (abs(leftY) > AXIS_LEFT_DEAD_ZONE) {
            this->camera->move(-leftY * distance);
        }

        float thetaInc = 0.0;
        float phiInc = 0.0;
        // Look using right stick
        if (abs(rightX) > AXIS_RIGHT_DEAD_ZONE) {
            thetaInc = rightX * AXIS_DELTA_THETA;
        }
        if (abs(rightY) > AXIS_RIGHT_DEAD_ZONE) {
            phiInc = rightY * AXIS_DELTA_PHI;
        }
        // Avoid Drift
        if (abs(rightX) > AXIS_TURN_THRESHOLD && abs(rightY) > AXIS_TURN_THRESHOLD) {
            this->camera->turn(thetaInc, phiInc);
        }
        if (buttonA) {
            this->camera->ascend(distance);
        }
        if (buttonB) {
            this->camera->ascend(-distance);
        }
        if (leftShoulder) {
            this->camera->roll(-DELTA_ROLL);
        }
        if (rightShoulder) {
            this->camera->roll(DELTA_ROLL);
        }
    }
}

void Visualiser::screenshot() {
    this->screenshot(false);
}

void Visualiser::screenshot(const bool verbose) {
    const char *SCREENSHOT_FILENAME = "screenshot.png";
    int w = 1;
    int h = 1;

    SDL_GetWindowSize(this->window, &w, &h);
    unsigned char *pixels = new unsigned char[w * h * 4];  // 4 bytes for RGBA;
    // Get data
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    const bool result = Texture::saveImage(pixels, w, h, SCREENSHOT_FILENAME);
    if (verbose) {
        if (result) {
            printf("Failed to write screenshot to '%s'\n", SCREENSHOT_FILENAME);
        } else {
            printf("Screenshot written to '%s'\n", SCREENSHOT_FILENAME);
        }
    }
    delete[] pixels;
}
void Visualiser::setWindowIcon() {
    if (!window)
        return;
    auto surface = Texture::loadImage(modelConfig.isPython ? "resources/pyflamegpu_icon.png" : "resources/flamegpu_icon.png");
    if (surface)
        SDL_SetWindowIcon(window, surface.get());
}
void Visualiser::updateDebugMenu() {
    if (debugMenu && debugMenu->getVisible()) {
        std::stringstream ss;
        ss << std::setprecision(3);
        ss << std::fixed;  // To stop camera floats from changing box width
        ss << "===Debug Menu===" << "\n";
        const glm::vec3 eye = camera->getEye();
        const glm::vec3 look = camera->getLook();
        const glm::vec3 up = camera->getUp();
        ss << "Camera Location: (" << eye.x  << ", " << eye.y << ", " << eye.z << ")" "\n";
        ss << "Camera Direction: (" << look.x << ", " << look.y << ", " << look.z << ")" "\n";
        ss << "Camera Up: (" << up.x << ", " << up.y << ", " << up.z << ")" "\n";
        ss << "MSAA: " << (this->msaaState ? "On" : "Off") << "\n";
        switch (this->fpsStatus) {
            case 2:
                ss << "Display FPS: Show All" << "\n";
            break;
            case 1:
                ss << "Display FPS: Show Step Count" << "\n";
                break;
            default:
                ss << "Display FPS: Off" << "\n";
        }
        ss << "Display Lines: " << (this->renderLines ? "On" : "Off") << "\n";
        ss << "Agent Populations:" << "\n";
        for (const auto &as : agentStates) {
            if (debugMenu_showStateNames) {
                ss << "  " << as.first.first << "(" << as.first.second << "): " << as.second.requiredSize << "\n";
            } else {
                ss << "  " << as.first.first << ": " << as.second.requiredSize << "\n";
            }
        }

        // Last item (don't end \n)
        ss << "Pause Simulation: " << (this->pause_guard ? "On" : "Off");
        debugMenu->setString(ss.str().c_str());
    }
}
