#include "config/ModelConfig.h"

ModelConfig::ModelConfig(const char* _windowTitle)
    : windowTitle(nullptr) {
    setString(&windowTitle, _windowTitle);
    windowDimensions[0] = 1280;
    windowDimensions[1] = 720;
    clearColor[0] = 0.0f;
    clearColor[1] = 0.0f;
    clearColor[2] = 0.0f;
}
ModelConfig::~ModelConfig() {
    if (windowTitle)
        free(const_cast<char*>(windowTitle));
}

ModelConfig::ModelConfig(const ModelConfig &other)
    : windowTitle(nullptr) {
    *this = other;
}
ModelConfig &ModelConfig::operator=(const ModelConfig &other) {
    setString(&windowTitle, other.windowTitle);
    memcpy(windowDimensions, other.windowDimensions, sizeof(windowDimensions));
    memcpy(clearColor, other.clearColor, sizeof(clearColor));
    return *this;
}
