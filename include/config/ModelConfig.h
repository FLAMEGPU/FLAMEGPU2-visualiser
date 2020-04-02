#ifndef INCLUDE_CONFIG_MODELCONFIG_H_
#define INCLUDE_CONFIG_MODELCONFIG_H_

#include <string>

/**
* This class holds the common components for a visualisation
*/
struct ModelConfig {
    ~ModelConfig();
    ModelConfig(const ModelConfig &other);
    ModelConfig &operator=(const ModelConfig &other);

    /**
    * Inline string to char* util
    */
 private:
    static void setString(const char ** target, const std::string &src) {
        if (*target)
            free(const_cast<char*>(*target));
        char *t = static_cast<char*>(malloc(src.size() + 1));
        snprintf(t, src.length(), "%s", src.c_str());
        *target = t;
    }
};

#endif  //  INCLUDE_CONFIG_MODELCONFIG_H_
