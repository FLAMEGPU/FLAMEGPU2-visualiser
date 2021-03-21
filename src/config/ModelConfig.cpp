#include "config/ModelConfig.h"

#include <cstring>
ModelConfig::StaticModel::StaticModel()
    : path("")
    , texture("")
    , scale{-1, 0, 0}
    , location{0, 0, 0}
    , rotation{1, 0, 0, 0} { }
ModelConfig::ModelConfig(const char* _windowTitle)
    : windowTitle(nullptr)
    , windowDimensions{1280, 720}
    , clearColor{0, 0, 0}
    , fpsVisible(true)
    , fpsColor{1, 1, 1}
    , cameraLocation{1.5f, 1.5f, 1.5f}
    , cameraTarget{0, 0, 0}
    , cameraSpeed{0.05f, 5}
    , nearFarClip{0.05f, 5000}
    , stepVisible(true)
    , beginPaused(false) {
    setString(&windowTitle, _windowTitle);
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
    beginPaused = other.beginPaused;
    //staticModels
    //lines
    return *this;
}
