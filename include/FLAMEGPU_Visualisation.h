#ifndef INCLUDE_FLAMEGPU_VISUALISATION_H_
#define INCLUDE_FLAMEGPU_VISUALISATION_H_

#include <map>
#include <string>

#include "config/TexBufferConfig.h"

struct AgentStateConfig;
struct ModelConfig;
class Visualiser;
class LockHolder;

/**
 * This class forms the interface between FLAMEGPU2 and FLAMEGPU2_Visualiser
 * It is important that it remains raw C (or any std is inline only)
 */
class FLAMEGPU_Visualisation {
 public:
    explicit FLAMEGPU_Visualisation(const ModelConfig &modelcfg);
    ~FLAMEGPU_Visualisation();
    void addAgentState(const std::string &agent_name, const std::string &state_name, const AgentStateConfig &vc,
         const std::map<TexBufferConfig::Function, TexBufferConfig>& core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& tex_buffers) {
        addAgentState(agent_name.c_str(), state_name.c_str(), vc, core_tex_buffers, tex_buffers);
    }
    void requestBufferResizes(const std::string &agent_name, const std::string &state_name, const unsigned int buffLen) {
        requestBufferResizes(agent_name.c_str(), state_name.c_str(), buffLen);
    }
    void lockMutex();
    void releaseMutex();
    void updateAgentStateBuffer(const std::string &agent_name, const std::string &state_name, const unsigned int buffLen,
        const std::map<TexBufferConfig::Function, TexBufferConfig>& core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& tex_buffers) {
        updateAgentStateBuffer(agent_name.c_str(), state_name.c_str(), buffLen, core_tex_buffers, tex_buffers);
    }
    void setStepCount(const unsigned int stepCount);
    /*
     * Start visualiser in background thread
     */
    void start();
    /**
     * Async kill visualiser
     */
    void stop();
    /**
     * Block thread until visualiser exits
     * @note This expects user to kill visualiser with cross in the corner
     */
    void join();
    /**
     *
     */
    bool isRunning() const;

 private:
    void addAgentState(const char *agent_name, const char *state_name, const AgentStateConfig &vc,
        const std::map<TexBufferConfig::Function, TexBufferConfig>& core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& tex_buffersz);
    void requestBufferResizes(const char *agent_name, const char *state_name, const unsigned int buffLen);
    void updateAgentStateBuffer(const char *agent_name, const char *state_name, const unsigned int buffLen,
        const std::map<TexBufferConfig::Function, TexBufferConfig>& core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& tex_buffers);
    Visualiser *vis = nullptr;
    LockHolder *lock = nullptr;
    /**
     * If non-0, limits the number of simulation steps per second, by blocking the return of releaseMutex
     * Blocking whilst the mutex was held, would block visualisation updates as that also uses the mutex
     * The actual value is the number of ms per step
     */
    unsigned int step_ms;
};

#endif  // INCLUDE_FLAMEGPU_VISUALISATION_H_
