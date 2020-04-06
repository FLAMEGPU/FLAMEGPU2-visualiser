#include "config/ModelConfig.h"

#include <cstring>

ModelConfig::ModelConfig(const char* _windowTitle)
    : windowTitle(nullptr)
    , fpsVisible(true) {
    setString(&windowTitle, _windowTitle);
    windowDimensions[0] = 1280;
    windowDimensions[1] = 720;
    clearColor[0] = 0.0f;
    clearColor[1] = 0.0f;
    clearColor[2] = 0.0f;
    fpsColor[0] = 1.0f;
    fpsColor[1] = 1.0f;
    fpsColor[2] = 1.0f;
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
    fpsVisible = other.fpsVisible;
    memcpy(fpsColor, other.fpsColor, sizeof(fpsColor));
    return *this;
}
