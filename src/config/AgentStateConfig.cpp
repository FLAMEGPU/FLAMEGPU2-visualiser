#include "config/AgentStateConfig.h"
#include "config/Stock.h"

namespace flamegpu {
namespace visualiser {

AgentStateConfig::AgentStateConfig()
    : model_path(nullptr)
    , model_texture(nullptr) {
    setString(&model_path, Stock::Models::ICOSPHERE.modelPath);
    model_scale[0] = -1.0f;  // Uniformly scale model to 1
}
AgentStateConfig::~AgentStateConfig() {
    if (model_path)
        free(const_cast<char*>(model_path));
    if (model_texture)
        free(const_cast<char*>(model_texture));
}
AgentStateConfig::AgentStateConfig(const AgentStateConfig &other)
    : model_path(nullptr)
    , model_texture(nullptr)  {
    *this = other;
}
AgentStateConfig &AgentStateConfig::operator=(const AgentStateConfig &other) {
    if (other.model_path)
        setString(&model_path, other.model_path);
    if (other.model_texture)
        setString(&model_texture, other.model_texture);
    memcpy(model_scale, other.model_scale, sizeof(model_scale));

    color_shader_src = other.color_shader_src;
    tex_buffers = other.tex_buffers;
    return *this;
}

}  // namespace visualiser
}  // namespace flamegpu
