#ifndef SRC_HUD_H_
#define SRC_HUD_H_

#include <list>
#include <tuple>
#include <memory>

#include <glm/glm.hpp>

#include "util/GLcheck.h"

class OverlayGroup;
class Overlay;

/*
Represents the orthographic plane covering the screen
Add/Remove overlays (e.g. Text, Sprite2D) to control where HUD elements are to be rendered.
*/
class HUD {
 public:
    enum class AnchorV { North, Center, South };
    enum class AnchorH { West, Center, East };
    /*
    Wraps Overlays which have been added to the HUD, to include details necessary for rendering them
    */
    class Item {
     public:
        /**
         * Convenience constructor
         * @param overlay The overlay element to be rendered as part of the HUD
         * @param x The horizontal offset from the anchored position
         * @param y The vertical offset from the anchored position
         * @param window_w The width of the HUD
         * @param window_h The height of the HUD
         * @param anchorV Vertical Anchoring location (North|Center|South), defaults Center
         * @param anchorH Horizontal Anchoring location (East|Center|West), defaults Center
         * @param zIndex The priority for which overlay should be on-top.
         */
        Item(std::shared_ptr<Overlay> overlay, const int &x, const int &y, const unsigned int &window_w, const unsigned int &window_h, AnchorV anchorV = AnchorV::Center, AnchorH anchorH = AnchorH::Center, const int &zIndex = 0);
        /**
         * Initialises the HUDItem, by calculating the elements position
         * @param overlay The overlay element to be rendered as part of the HUD
         * @param offset The  offset from the anchored position
         * @param windowDims The width and height of the HUD
         * @param anchorV Vertical Anchoring location (North|Center|South), defaults Center
         * @param anchorH Horizontal Anchoring location (East|Center|West), defaults Center
         * @param zIndex The priority for which overlay should be on-top.
         */
        Item(std::shared_ptr<Overlay> overlay, const glm::ivec2 &offset, const glm::uvec2 &windowDims, AnchorV anchorV = AnchorV::Center, AnchorH anchorH = AnchorH::Center, const int &zIndex = 0);
        /**
         * Convenience resizeWindow(const glm::uvec2 &)
         * @param w The new window width
         * @param h The new window height
         */
        void resizeWindow(const unsigned int &w, const unsigned int &h) { resizeWindow({ w, h }); }
        /**
         * Update the overlays quad location, based on new window size, anchors, offsets and overlay dimensions
         * @param dims The new window width and height
         * @note if glm::uvec2(0) is passed, the previous value will be used (this allows overlays to trigger themselves.
         */
        void resizeWindow(const glm::uvec2 &dims = glm::uvec2(0));
        std::shared_ptr<Overlay> overlay;
        const glm::ivec2 offset;
        const AnchorV anchorV;
        const AnchorH anchorH;
        const int zIndex;
        GLuint vbo;
        GLuint fvbo;
        void *data;
    };
    /**
     * Convenience constructor
     * @param width Window width
     * @param height Window height
     * @note This is done within Visualisation, regular users have no reason to instaniate a HUD
     */
    HUD(const unsigned int &width, const unsigned int &height);
    /**
     * Creates a new HUD, specifying the window dimensions
     * @param dims Window width and height
     * @note This is done within Visualisation, regular users have no reason to instaniate a HUD
     */
    explicit HUD(const glm::uvec2 &dims);
    /**
     * Adds an overlay element to the HUD
     * @param overlay The overlay element to be rendered as part of the HUD
     * @param anchorV Vertical Anchoring location (North|Center|South), defaults Center
     * @param anchorH Horizontal Anchoring location (East|Center|West), defaults Center
     * @param offset The horizontal and vertical offsets from the anchored position
     * @param zIndex The priority for which overlay should be on-top.
     * @note If two items share the same z-index, the new item will insert as though it has the lower z-index (and be rendered underneath)
     * @note Adding the same overlay to HUD a second time, will remove it's first instance.
     */
    void add(std::shared_ptr<Overlay> overlay, AnchorV anchorV = AnchorV::Center, AnchorH anchorH = AnchorH::Center, const glm::ivec2 &offset = glm::ivec2(0), int zIndex = 0);
    /**
     * Adds a collection of overlays element to the HUD
     * @param overlays The group of overlay elements to be rendered as part of the HUD
     * @param anchorV Vertical Anchoring location (North|Center|South), defaults Center
     * @param anchorH Horizontal Anchoring location (East|Center|West), defaults Center
     * @param offset The horizontal and vertical offsets from the anchored position (This will be added to the sub-offsets of the overlays)
     * @param zIndex The priority for which the overlays should be on-top (This will be added to the sub-offsets of the overlays)
     * @note If two items share the same z-index, the new item will insert as though it has the lower z-index (and be rendered underneath)
     * @note Adding the same overlay group to HUD a second time, will remove it's first instance.
     */
    void add(std::shared_ptr <OverlayGroup> overlays, AnchorV anchorV = AnchorV::Center, AnchorH anchorH = AnchorH::Center, const glm::ivec2 &offset = glm::ivec2(0), int zIndex = 0);
    /**
     * Removes the specified overlay from the HUD
     * @return The number of overlays removed
     */
    unsigned int remove(std::shared_ptr<Overlay> overlay);
    /**
     * Removes the specified overlay group from the HUD
     * @return The number of individual overlays removed
     */
    unsigned int remove(std::shared_ptr<OverlayGroup> overlays);
    /**
     * all overlays from the HUD
     */
    void clear();
    /**
     * Returns the number of overlays currently on the HUD
     * @return The number of overlays
     */
    unsigned int getCount();
    /**
     * Calls reload on all held overlay elements
     * @note Some overlay subclasses may not implement reload, however their shaders will be reloaded
     */
    void reload();
    /**
     * Renders all HUD elements in reverse z-index order, with GL_DEPTH_TEST disabled
     */
    void render();
    /**
     * Convenience resizeWindow(const glm::uvec2 &)
     * @param width New window width
     * @param height New window height
     */
    void resizeWindow(const unsigned int &width, const unsigned int &height) { resizeWindow(glm::uvec2(width, height)); }
    /**
     * Repositions all HUD ovlerays according to the new window dimensions and their repsective anchoring/offset args
     * @param dims New window width and height
     */
    void resizeWindow(const glm::uvec2 &dims);

 private:
    const glm::mat4 modelViewMat;
    glm::mat4 projectionMat;
    // Holds the overlay elements to be rendered in reverse z-index order
    std::list<std::shared_ptr<Item>> stack;
    glm::uvec2 dims;
};

#endif  // SRC_HUD_H_
