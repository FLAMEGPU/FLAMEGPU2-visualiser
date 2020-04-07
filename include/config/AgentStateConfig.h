#ifndef INCLUDE_CONFIG_AGENTSTATECONFIG_H_
#define INCLUDE_CONFIG_AGENTSTATECONFIG_H_

#include <string>

/**
* This class holds the common components for a visualisation
*/
struct AgentStateConfig {
    friend class AgentVis;
    friend class AgentStateVis;
    AgentStateConfig();
    ~AgentStateConfig();
    AgentStateConfig(const AgentStateConfig &other);
    AgentStateConfig &operator=(const AgentStateConfig &other);

    const char *model_path = nullptr;
    float model_scale[3];

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
