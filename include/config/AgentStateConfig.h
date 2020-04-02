#ifndef INCLUDE_CONFIG_AGENTSTATECONFIG_H_
#define INCLUDE_CONFIG_AGENTSTATECONFIG_H_

#include <string>

/**
* This class holds the common components for a visualisation
*/
struct AgentStateConfig {
    AgentStateConfig();
    ~AgentStateConfig();
    AgentStateConfig(const AgentStateConfig &other);
    AgentStateConfig &operator=(const AgentStateConfig &other);

    const char *model_path = nullptr;  // "C:\\Users\\Robadob\\fgpu2_visualiser\\resources\\icosphere.obj"

    /**
    * Inline string to char* util
    */
    void setModel(const std::string &_model_path) {
        setString(&model_path, _model_path);
    }

 private:
    static void setString(const char ** target, const std::string &src) {
        if (*target)
            free(const_cast<char*>(*target));
        char *t = static_cast<char*>(malloc(src.size() + 1));
        snprintf(t, src.size() + 1, "%s", src.c_str());
        *target = t;
    }
};

#endif  //  INCLUDE_CONFIG_AGENTSTATECONFIG_H_
