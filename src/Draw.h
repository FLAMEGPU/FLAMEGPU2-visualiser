#ifndef SRC_DRAW_H_
#define SRC_DRAW_H_
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <unordered_map>

#include "interface/Renderable.h"
#include "shader/Shaders.h"

/**
 * Class for automatically managing VAO's and VBO's for drawing points, lines and polylines at runtime
 * If draw routines include a string when opened, they be recalled which is more performant than recreating them from scratch if drawing static structures
 */
class Draw : Renderable {
 public:
    enum Type {
        Points,
        Lines,
        Polyline,
    };

 private:
    /**
     * Holds information required to draw primitive point/line structures
     */
    struct State {
        Type mType;    // Drawing type (point, line, polyline)
        unsigned int count;    // Number of vertices
        unsigned int offset;  // Offset into the vbo (in vertices)
        float mWidth;        // Pen width
    };

 public:
    /**
     * Create a draw object
     * @param bufferLength Optionally specify the number of vertices the vbo will be required to store
     * @param initialColor Optionally specify the initial draw color
     * @param initialWidth Optionally specify the initial draw width
     * @note If bufferLength is exceeded, the buffer will resize according to STORAGE_MUTLIPLIER
     */
    explicit Draw(const unsigned int &bufferLength = 0, const glm::vec4 &initialColor = glm::vec4(1), const float &initialWidth = 1.0f);
    /**
     * Deletes VBOs
     */
    ~Draw();
    /**
     * Begins a draw state
     * @param type Type of drawing to create
     * @param name Optional name for the drawing to be used for recall
     * @throw Throws a runtime exception if called whilst the draw state is already open
     */
    void begin(Type type, const std::string &name = std::string());
    /**
     * Ends a drawing and calls render code
     * @param replaceExisting This value must be set to true if using a named draw state which is to overwrite an existing named draw state
     * @throw Throws a runtime exception if called whilst the draw state is not open
     */
    void end(bool replaceExisting = false);
    /**
     * Ends a drawing and saves it (only usable with named draw state)
     * @param replaceExisting Explicitly confirms intention to overwrite existing draw state
     * @throw Throws a runtime exception if called whilst the draw state is not open
     */
    void save(bool replaceExisting = false);
    /**
     * Renders a saved drawstate
     * @param name The name of the draw state to render
     */
    void render(const std::string &name);
    /**
     * Reloads the shader
     */
    void reload() override;
    /**
     * Provides view matrix to the shader
     * @param viewMat Set the view matrix to be used for rendering
     */
    void setViewMatPtr(const glm::mat4 *viewMat) override;
    /**
     * Provides projection matrix to the shader
     * @param projectionMat Set the projection matrix to be used for rendering
     */
    void setProjectionMatPtr(const glm::mat4 *projectionMat) override;
    /**
     * Provides lights buffer to the shader
     * @param bufferBindingPoint Set the buffer binding point to be used for rendering
     */
    void setLightsBuffer(const GLuint &bufferBindingPoint) override;
    /**
     * Sets the width to use for the current drawing
     * The last value set before end() or save() will be taken
     */
    void width(const float &w) { tWidth = w; }
    /**
     * Sets the colour for subsequent vertices
     * @param c Colour
     */
    void color(const glm::vec3 &c) { tColor = glm::vec4(c, 1.0f); }
    /**
     * Sets the colour for subsequent vertices, treating alpha as 1.0f
     * @param c Colour
     */
    void color(const glm::vec4 &c) { tColor = c; }
    /**
     * Sets the colour for subsequent vertices, treating alpha as 1.0f
     * @param r red
     * @param g green
     * @param b blue
     */
    void color(const float &r, const float &g, const float &b, const float &a = 1.0f) { tColor = glm::vec4(r, g, b, a); }
    /**
     * Adds a new vertex to the draw state
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     */
    void vertex(const float &x, const float &y = 0.0f, const float &z = 0.0f) { vertex(glm::vec3(x, y, z)); }
    /**
     * Adds a new vertex to the draw state
     * @param v vertex
     * @note b component of glm::vec2 will be treated as 0.0f
     * @note w component of glm::vec4 will be ignored
     */
    void vertex(const glm::vec2 &v) { vertex(glm::vec3(v, 0.0f)); }
    void vertex(const glm::vec3 &v) {
        if (!isDrawing)
            throw std::runtime_error("Draw::vertex() may only be called between Draw::begin() and Draw::end() or Draw::save()\n");
        tVertices.push_back(v);
        tColors.push_back(tColor);
    }
    void vertex(const glm::vec4 &v) { vertex(glm::vec3(v)); }
    /**
    * Adds a new vertex to the draw state from a pointer
    * Vertices are treated at glm::vec3, absent components will be treated as 0.0f
    * @param v pointer to vertex data
    */
    void vertex1(const float *v) { vertex(glm::vec3(v[0], 0.0f, 0.0f)); }
    void vertex2(const float *v) { vertex(glm::vec3(v[0], v[1], 0.0f)); }
    void vertex2(const glm::vec2 *v) { vertex(glm::vec3(v[0], 0.0f)); }
    void vertex2(const glm::vec3 *v) { vertex(v[0]); }
    void vertex2(const glm::vec4 *v) { vertex(glm::vec3(v[0])); }
    void vertex3(const float *v) { vertex(glm::vec3(v[0], 0.0f, 0.0f)); }
    void vertex3(const glm::vec2 *v) { vertex(glm::vec3(*v, 0.0f)); }
    void vertex3(const glm::vec3 *v) { vertex(v[0]); }
    void vertex3(const glm::vec4 *v) { vertex(glm::vec3(v[0])); }

 private:
    /**
     * Resizes the vbo to newLength and defragments the old data
     * @param newLength The number of vertices the vbo can support
     */
    void resize(unsigned int newLength);
    /*
     * Unused, defragments vbo to remove gaps
     */
    void defragment() { resize(vboLen); }
    /*
     * Unused, compacts vbo to remove all gaps and additional space
     */
    void compact() { resize(requiredLength); }
    /**
     * Renders the provided draw state
     * @param state The draw state to be rendered
     */
    void render(const State &state) const;
    /**
     * Holds all created draw states
     */
    std::unordered_map<std::string, State> stateDirectory;
    /**
     * Holds a list of pairs stating where gaps can be found in the current vbo
     * @note Pair structure is {offset, count}
     */
    std::vector<std::pair<unsigned int, unsigned int>> vboGaps;
    /*
     * Saves the current in progress draw state returning the create draw state struct
     * @param isTemporary Specifies whether the create state is temporary (passing false will cause gapList to be updated)
     */
    State _save(bool isTemporary);
    /*
     * Flags whether we are currently drawing
     */
    bool isDrawing = false;
    std::string tName;
    glm::vec4 tColor;
    float tWidth;
    Type tType;
    std::vector<glm::vec3> tVertices;
    std::vector<glm::vec4> tColors;
    /**
     * Data required for rendering
     */
    std::shared_ptr<Shaders> shaders;
    Shaders::VertexAttributeDetail vertices, colors;
    /**
     * Data required for managing storage
     */
    unsigned int vboLen, vboOffset;
    static const unsigned int DEFAULT_INITIAL_VBO_LENGTH;
    static const float STORAGE_MUTLIPLIER;
    unsigned int requiredLength;
    /**
     * @return The GLenum which matches Type t
     */
    static GLenum toGL(const Type &t);
    /**
     * @return calls glPointSize(w) or glLineWidth(w) correpsonding to Type t
     */
    static void setWidth(const Type &t, const float &w);
    /**
     * @return calls glPointSize(1) or glLineWidth(1) correpsonding to Type t
     */
    static void clearWidth(const Type &t);
};
#endif  // SRC_DRAW_H_
