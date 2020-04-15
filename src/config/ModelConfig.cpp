#include "config/ModelConfig.h"

#include <cstring>

ModelConfig::ModelConfig(const char* _windowTitle)
    : windowTitle(nullptr)
    , fpsVisible(true)
    , stepVisible(true) {
    setString(&windowTitle, _windowTitle);
    windowDimensions[0] = 1280;
    windowDimensions[1] = 720;
    clearColor[0] = 0.0f;
    clearColor[1] = 0.0f;
    clearColor[2] = 0.0f;
    fpsColor[0] = 1.0f;
    fpsColor[1] = 1.0f;
    fpsColor[2] = 1.0f;
    cameraLocation[0] = 1.5f;
    cameraLocation[1] = 1.5f;
    cameraLocation[2] = 1.5f;
    cameraTarget[0] = 0.0f;
    cameraTarget[1] = 0.0f;
    cameraTarget[2] = 0.0f;
    cameraSpeed[0] = 0.05f;
    cameraSpeed[1] = 5.0f;
    nearFarClip[0] = 0.05f;
    nearFarClip[1] = 5000.0f;
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
    memcpy(cameraLocation, other.cameraLocation, sizeof(cameraLocation));
    memcpy(cameraTarget, other.cameraTarget, sizeof(cameraTarget));
    memcpy(cameraSpeed, other.cameraSpeed, sizeof(cameraSpeed));
    memcpy(nearFarClip, other.nearFarClip, sizeof(nearFarClip));
    stepVisible = other.stepVisible;
    return *this;
}
