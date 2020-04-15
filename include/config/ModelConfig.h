#ifndef INCLUDE_CONFIG_MODELCONFIG_H_
#define INCLUDE_CONFIG_MODELCONFIG_H_

#include <string>

/**
* This class holds the common components for a visualisation
*/
struct ModelConfig {
    friend class ModelVis;
    explicit ModelConfig(const char *windowTitle);
    ~ModelConfig();
    ModelConfig(const ModelConfig &other);
    ModelConfig &operator=(const ModelConfig &other);

    const char* windowTitle;
    unsigned int windowDimensions[2];
    float clearColor[3];
    bool fpsVisible;
    float fpsColor[3];
    float cameraLocation[3];
    float cameraTarget[3];
    float cameraSpeed[2];
    float nearFarClip[2];

 private:
     /**
      * Inline string to char* util
      */
    static void setString(const char ** target, const std::string &src) {
        if (*target)
            free(const_cast<char*>(*target));
        char *t = static_cast<char*>(malloc(src.size() + 1));
        snprintf(t, src.size() + 1, "%s", src.c_str());
        *target = t;
    }
};

#endif  //  INCLUDE_CONFIG_MODELCONFIG_H_
