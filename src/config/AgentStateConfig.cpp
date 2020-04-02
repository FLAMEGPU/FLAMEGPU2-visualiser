#include "config/AgentStateConfig.h"

AgentStateConfig::AgentStateConfig()
    : model_path(nullptr) {
    setModel("C:\\Users\\Robadob\\fgpu2_visualiser\\resources\\icosphere.obj");
}
AgentStateConfig::~AgentStateConfig() {
    if (model_path)
        free(const_cast<char*>(model_path));
}
AgentStateConfig::AgentStateConfig(const AgentStateConfig &other) {
    if (other.model_path)
        setModel(other.model_path);
}
AgentStateConfig &AgentStateConfig::operator=(const AgentStateConfig &other) {
    if (other.model_path)
        setModel(other.model_path);
    return *this;
}
