#ifndef INCLUDE_FLAMEGPU_VISUALISER_CONFIG_IMGUIWIDGETS_H_
#define INCLUDE_FLAMEGPU_VISUALISER_CONFIG_IMGUIWIDGETS_H_

#include <imgui.h>

#include <memory>
#include <string>
#include <cstdint>
#include <algorithm>  // Not actually required, linter just thinks initialiser for max is fn call

namespace flamegpu {
namespace visualiser {

/**
 * Template for converting a type to the corresponding ImGui type enum
 * Useful when summing unknown values
 * e.g. sum_input_t<float>::result_t == double
 * e.g. sum_input_t<uint8_t>::result_t == uint64_t
 */
template <typename T> struct imgui_type;
/**
 * @see imgui_type
 */
template <> struct imgui_type<char>     { static constexpr ImGuiDataType_ t = ImGuiDataType_S8;     static constexpr const char* fmt = "%c"; };
template <> struct imgui_type<int8_t>   { static constexpr ImGuiDataType_ t = ImGuiDataType_S8;     static constexpr const char* fmt = "%hhd"; };
template <> struct imgui_type<uint8_t>  { static constexpr ImGuiDataType_ t = ImGuiDataType_U8;     static constexpr const char* fmt = "%hhu"; };
template <> struct imgui_type<int16_t>  { static constexpr ImGuiDataType_ t = ImGuiDataType_S16;    static constexpr const char* fmt = "%hd"; };
template <> struct imgui_type<uint16_t> { static constexpr ImGuiDataType_ t = ImGuiDataType_U16;    static constexpr const char* fmt = "%hu"; };
template <> struct imgui_type<int32_t>  { static constexpr ImGuiDataType_ t = ImGuiDataType_S32;    static constexpr const char* fmt = "%d"; };
template <> struct imgui_type<uint32_t> { static constexpr ImGuiDataType_ t = ImGuiDataType_U32;    static constexpr const char* fmt = "%u"; };
template <> struct imgui_type<int64_t>  { static constexpr ImGuiDataType_ t = ImGuiDataType_S64;    static constexpr const char* fmt = "%lld"; };
template <> struct imgui_type<uint64_t> { static constexpr ImGuiDataType_ t = ImGuiDataType_U64;    static constexpr const char* fmt = "%llu"; };
template <> struct imgui_type<float>    { static constexpr ImGuiDataType_ t = ImGuiDataType_Float;  static constexpr const char* fmt = "%g"; };
template <> struct imgui_type<double>   { static constexpr ImGuiDataType_ t = ImGuiDataType_Double; static constexpr const char* fmt = "%g"; };

/**
 * The most generic abstract ImGui element superclass
 * All other elements inherit from this
 */
struct PanelElement {
    virtual ~PanelElement() = default;
    /**
     * Triggers the underlying call to ImGui
     */
    virtual bool addToImGui() = 0;
    /**
     * Provides copy construction behaviour
     */
    virtual std::unique_ptr<PanelElement> clone() const = 0;
};
/**
 * Separator element, creates a horizontal line between elements
 */
struct SeparatorElement : PanelElement {
    /**
     * Constructor
     */
    SeparatorElement() { }
    bool addToImGui() override { ImGui::Separator(); return false; }
    [[nodiscard]] std::unique_ptr<PanelElement> clone() const override { return std::unique_ptr<PanelElement>(new SeparatorElement()); }
};
/**
 * Merely denotes that return value should be treated differently
 */
struct SectionElement : PanelElement { };
/**
 * Collapsible header element, creates a titled section which can be collapsed
 */
struct HeaderElement : SectionElement {
    /**
     * Constructor
     * @param _text Text displayed in the section header
     * @param _begin_open If true, the section will not begin in a collapsed state
     */
    explicit HeaderElement(const std::string& _text, const bool _begin_open)
        : text(_text)
        , begin_open(_begin_open) { }
    bool addToImGui() override { return ImGui::CollapsingHeader(text.c_str(), begin_open ? ImGuiTreeNodeFlags_DefaultOpen : 0); }
    [[nodiscard]] std::unique_ptr<PanelElement> clone() const override { return std::unique_ptr<PanelElement>(new HeaderElement(text, begin_open)); }
    std::string text;
    bool begin_open;
};
/**
 * Ends a section, unhiding following elements
 */
struct EndSectionElement : SectionElement {
    /**
     * Constructor
     */
    EndSectionElement() { }
    bool addToImGui() override { return true; }
    [[nodiscard]] std::unique_ptr<PanelElement> clone() const override { return std::unique_ptr<EndSectionElement>(new EndSectionElement()); }
    std::string text;
};
/**
 * Label element, only contains a string used to hold it's text to be displayed
 */
struct LabelElement : PanelElement {
    /**
     * Constructor
     * @param _text Text displayed on the label
     */
    explicit LabelElement(const std::string &_text)
        : text(_text) { }
    bool addToImGui() override { ImGui::Text("%s", text.c_str()); return false; }
    [[nodiscard]] std::unique_ptr<PanelElement> clone() const override { return std::unique_ptr<PanelElement>(new LabelElement(text)); }
    std::string text;
};
/**
 * Environment property modifier generic
 * All environment properties inherit from this
 */
struct EnvPropertyElement : PanelElement {
    /**
     * Constructor
     * @param _name Name of the environment property
     * @param _index Index of the environment property element (0 if the property is not an array property)
     * @param _ptr_offset Pointer offset from the environment property origin (as this depending on the index and type size, 0 if the property is not an array property)
     * @note _ptr_offset should be calculated using the type information available to the subclass
     */
    explicit EnvPropertyElement(const std::string& _name, unsigned int _index, unsigned int _ptr_offset)
        : data_ptr(nullptr)
        , name(_name)
        , index(_index)
        , ptr_offset(_ptr_offset) { }
    void* data_ptr;
    std::string name;
    unsigned int index;
    unsigned int ptr_offset;
    bool is_const = false;
    bool addToImGui() override = 0;
    std::unique_ptr<PanelElement> clone() const override = 0;
    const std::string& getName() const { return name; }
    void setPtr(void *ptr) { data_ptr = static_cast<char*>(ptr) + ptr_offset; }
    void setConst(bool _is_const) { is_const = _is_const; }
    void setArray() { name += "[" + std::to_string(index) +"]"; }
};
/**
 * ImGui slider element for an environment property
 */
template<typename T>
struct EnvPropertySlider : EnvPropertyElement {
    /**
     * Constructor
     * @param _name Name of the environment property
     * @param _index Index of the environment property element (0 if the property is not an array property)
     * @param _min Minimum value of the slider
     * @param _max Maximum value of the slider
     */
    EnvPropertySlider(const std::string& _name, unsigned int _index, T _min, T _max)
        : EnvPropertyElement(_name, _index, _index * sizeof(T))
        , min(_min)
        , max(_max) { }
    T min, max;
    bool addToImGui() override {
        if (this->data_ptr) {
            if (this->is_const) ImGui::BeginDisabled();
            const bool rtn = ImGui::SliderScalar(this->name.c_str(), imgui_type<T>::t, this->data_ptr, &this->min, &this->max, imgui_type<T>::fmt, ImGuiSliderFlags_AlwaysClamp);
            if (this->is_const) ImGui::EndDisabled();
            return rtn;
        }
        ImGui::Text("Loading...%s", this->name.c_str());
        return false;
    }
    [[nodiscard]] std::unique_ptr<PanelElement> clone() const override { return std::unique_ptr<PanelElement>(new EnvPropertySlider<T>(this->name, this->index, this->min, this->max)); }
};
/**
 * ImGui drag element for an environment property
 */
template<typename T>
struct EnvPropertyDrag : EnvPropertyElement {
    /**
     * Constructor
     * @param _name Name of the environment property
     * @param _index Index of the environment property element (0 if the property is not an array property)
     * @param _min Minimum value that can be set
     * @param _max Maximum value that can be set
     * @param _speed Amount the value changes per pixel dragged
     */
    EnvPropertyDrag(const std::string& _name, unsigned int _index, T _min, T _max, float _speed)
        : EnvPropertyElement(_name, _index, _index * sizeof(T))
        , min(_min)
        , max(_max)
        , speed(_speed) { }
    T min, max;
    float speed;
    bool addToImGui() override {
        if (this->data_ptr) {
            if (this->is_const) ImGui::BeginDisabled();
            const bool rtn = ImGui::DragScalar(this->name.c_str(), imgui_type<T>::t, this->data_ptr, this->speed, &this->min, &this->max, imgui_type<T>::fmt, ImGuiSliderFlags_AlwaysClamp);
            if (this->is_const) ImGui::EndDisabled();
            return rtn;
        }
        ImGui::Text("Loading...%s", this->name.c_str());
        return false;
    }
    [[nodiscard]] std::unique_ptr<PanelElement> clone() const override { return std::unique_ptr<PanelElement>(new EnvPropertyDrag<T>(this->name, this->index, this->min, this->max, this->speed)); }
};
/**
 * ImGui input with +/- step buttons for an environment property
 */
template<typename T>
struct EnvPropertyInput : EnvPropertyElement {
    /**
     * Constructor
     * @param _name Name of the environment property
     * @param _index Index of the environment property element (0 if the property is not an array property)
     * @param _step Change per button click
     * @param _step_fast Change per tick when holding button (?)
     */
    EnvPropertyInput(const std::string& _name, unsigned int _index, T _step, T _step_fast)
        : EnvPropertyElement(_name, _index, _index * sizeof(T))
        , step(_step)
        , step_fast(_step_fast) { }
    T step, step_fast;
    bool addToImGui() override {
        if (this->data_ptr) {
            if (this->is_const) ImGui::BeginDisabled();
            const bool rtn = ImGui::InputScalar(this->name.c_str(), imgui_type<T>::t, this->data_ptr, &this->step, &this->step_fast, imgui_type<T>::fmt, ImGuiInputTextFlags_CallbackCompletion);
            if (this->is_const) ImGui::EndDisabled();
            return rtn;
        }
        ImGui::Text("Loading...%s", this->name.c_str());
        return false;
    }
    [[nodiscard]] std::unique_ptr<PanelElement> clone() const override { return std::unique_ptr<PanelElement>(new EnvPropertyInput<T>(this->name, this->index, this->step, this->step_fast)); }
};
/**
 * ImGui checkbox for integer type environment properties
 */
template<typename T>
struct EnvPropertyToggle : EnvPropertyElement {
    /**
     * Constructor
     * @param _name Name of the environment property
     * @param _index Index of the environment property element (0 if the property is not an array property)
     */
    explicit EnvPropertyToggle(const std::string& _name, unsigned int _index)
        : EnvPropertyElement(_name, _index, _index * sizeof(T))
        , data(false) { }
    bool data;
    bool addToImGui() override {
        if (this->data_ptr) {
            if (is_const) ImGui::BeginDisabled();
            // How to sync data
            if (ImGui::Checkbox(this->name.c_str(), &data)) {
                *static_cast<T*>(data_ptr) = static_cast<T>(data ? 1 : 0);
                return true;
            }
            // Really not sure that this sync will work as desired if the model updates the value independently
            data = *static_cast<T*>(data_ptr);
            if (is_const) ImGui::EndDisabled();
            return false;
        }
        ImGui::Text("Loading...%s", this->name.c_str());
        return false;
    }
    [[nodiscard]] std::unique_ptr<PanelElement> clone() const override { return std::unique_ptr<PanelElement>(new EnvPropertyToggle<T>(this->name, this->index)); }
};
}  // namespace visualiser
}  // namespace flamegpu

#endif  // INCLUDE_FLAMEGPU_VISUALISER_CONFIG_IMGUIWIDGETS_H_
