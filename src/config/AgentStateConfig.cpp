#include "config/AgentStateConfig.h"
#include "config/Stock.h"

AgentStateConfig::AgentStateConfig()
    : model_path(nullptr) {
    setString(&model_path , Stock::Models::ICOSPHERE.modelPath);
    model_scale[0] = -1.0f;  // Uniformly scale model to 1
}
AgentStateConfig::~AgentStateConfig() {
    if (model_path)
        free(const_cast<char*>(model_path));
}
AgentStateConfig::AgentStateConfig(const AgentStateConfig &other)
    : model_path(nullptr) {
    *this = other;
}
AgentStateConfig &AgentStateConfig::operator=(const AgentStateConfig &other) {
    if (other.model_path)
        setString(&model_path, other.model_path);
    memcpy(model_scale, other.model_scale, sizeof(model_scale));
    return *this;
}
