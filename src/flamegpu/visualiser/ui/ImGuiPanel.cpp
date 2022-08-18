#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <memory>
#include <string>
#include <map>
#include <cinttypes>  // for PRIu64 (cross-platform uint64_t format specifier)

#include "flamegpu/visualiser/Visualiser.h"
#include "flamegpu/visualiser/ui/ImGuiPanel.h"
#include "flamegpu/visualiser/shader/Shaders.h"

namespace flamegpu {
namespace visualiser {

ImGuiPanel::ImGuiPanel(const std::map<std::string, std::shared_ptr<PanelConfig>>& cfgs, const Visualiser& _vis)
    : Overlay(std::make_shared<Shaders>(Stock::Shaders::SPRITE2D))
    , vis(_vis) {
    for (const  auto &cfg : cfgs)
        configs.push_back(*cfg.second);
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();  // This can't be called in the render thread, as it tries to grab the window dimensions via SDL
}
void ImGuiPanel::reload() {
    first_render = 0;
}
void ImGuiPanel::drawPanel() {
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImVec2 prev_window_size(0, 0), prev_window_pos(main_viewport->WorkPos.x, main_viewport->WorkPos.y);
    for (const auto &config : configs) {
        // Translucent background
        ImGui::SetNextWindowBgAlpha(150.0f / 255.0f);
        // Size panel to fit it's content
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
        if (first_render < 2) {
            // Manual set once condition, as I can't get ImGuiCond to play nicely here for multiple panels
            // Position multiple panels so that they don't overlap
            ImGui::SetNextWindowPos(ImVec2(prev_window_pos.x + prev_window_size.x + 5, main_viewport->WorkPos.y + 5), ImGuiCond_Always);
        }

        // Actually start creating the panel
        if (!ImGui::Begin(config.title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
            prev_window_size = ImGui::GetWindowSize();
            prev_window_pos = ImGui::GetWindowPos();
            // Early out if the window is collapsed, as an optimization.
            ImGui::End();
            return;
        }

        // Give items a fixed width (I think this i supposed to be text, but possibly conflicts with auto window resize)
        ImGui::PushItemWidth(ImGui::GetFontSize() * 15);

        // Add the user defined items in order
        bool open = true;
        for (auto& e : config.ui_elements) {
            if (auto a = dynamic_cast<SectionElement*>(e.get())) {
                open = a->addToImGui();
            } else if (open) {
                e->addToImGui();
            }
        }

        // Finalise the panel
        ImGui::PopItemWidth();
        prev_window_size = ImGui::GetWindowSize();
        prev_window_pos = ImGui::GetWindowPos();
        ImGui::End();
    }
    first_render += first_render < 2 ? 1: 0;  // Auto size is not calculated until it's been drawn, so wait a frame to fix it
}
void ImGuiPanel::drawDebugPanel() const {
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 5, main_viewport->WorkPos.y + 5), ImGuiCond_Once);
    // Translucent background
    ImGui::SetNextWindowBgAlpha(150.0f / 255.0f);
    // Size panel to fit it's content (might fix this width so it doesn't jump about with camera mvmt)
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);

    // Actually start creating the panel
    ImGui::Begin("Debug Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    // Give items a fixed width (I think this i supposed to be text, but possibly conflicts with auto window resize)
    ImGui::PushItemWidth(ImGui::GetFontSize() * 15);

    ImGui::Text("Initial Random Seed: %" PRIu64, vis.randomSeed);
    const glm::vec3 eye = vis.camera->getEye();
    const glm::vec3 look = vis.camera->getLook();
    const glm::vec3 up = vis.camera->getUp();
    ImGui::Text("Camera Location : (% .3f, % .3f, % .3f)", eye.x, eye.y, eye.z);
    ImGui::Text("Camera Direction : (% .3f, % .3f, % .3f)", look.x, look.y, look.z);
    ImGui::Text("Camera Up : (% .3f, % .3f, % .3f)", up.x, up.y, up.z);
    ImGui::Text("MSAA: %s", (vis.msaaState ? "On" : "Off"));
    switch (vis.fpsStatus) {
        case 2:
            ImGui::Text("Display FPS: Show All");
            break;
        case 1:
            ImGui::Text("Display FPS: Show Step Count");
            break;
        default:
            ImGui::Text("Display FPS: Off");
    }
    ImGui::Text("Pause Simulation: %s", (vis.pause_guard ? "On" : "Off"));
    ImGui::Separator();
    ImGui::Text("Agent Populations:");
    for (const auto& as : vis.agentStates) {
        if (vis.debugMenu_showStateNames) {
            ImGui::BulletText("%s (%s): %u", as.first.first.c_str(), as.first.second.c_str(), as.second.requiredSize);
        } else {
            ImGui::BulletText("%s: %u", as.first.first.c_str(), as.second.requiredSize);
        }
    }

    // Finalise the panel
    ImGui::PopItemWidth();
    ImGui::End();
}
void ImGuiPanel::render(const glm::mat4*, const glm::mat4*, GLuint fbo) {
    ImGui::NewFrame();
    // bool is_true = true;
    // ImGui::ShowDemoWindow(&is_true);
    if (ui_visible) drawPanel();
    if (debug_menu_visible) drawDebugPanel();
    // This renders the interface to a host texture
    ImGui::Render();
    // This presumably copies the host texture to a GPU texture and renders it
    // imgui wants to be it's own HUD, so we kind of need to give it a full screen overlay to draw into
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // SDL_GL_SwapWindow(window);
}
void ImGuiPanel::registerProperty(const std::string& name, void* ptr, bool is_const, bool is_array) {
    for (const auto& config : configs) {
        for (auto& e : config.ui_elements) {
            if (auto a = dynamic_cast<EnvPropertyElement*>(e.get())) {
                if (a->getName() == name) {
                    if (is_const) a->setConst(true);
                    if (is_array) a->setArray();
                    a->setPtr(ptr);
                }
            }
        }
    }
    first_render = 0;  // As these can change window width
}

}  // namespace visualiser
}  // namespace flamegpu
