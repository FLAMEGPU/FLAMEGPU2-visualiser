#ifndef INCLUDE_FLAMEGPU_VISUALISER_FLAMEGPU_VISUALISATION_H_
#define INCLUDE_FLAMEGPU_VISUALISER_FLAMEGPU_VISUALISATION_H_

#include <map>
#include <string>
#include <cstdint>

#include "flamegpu/visualiser/config/TexBufferConfig.h"

namespace flamegpu {
namespace visualiser {

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
    void requestBufferResizes(const std::string &agent_name, const std::string &state_name, const unsigned int buffLen, bool force) {
        requestBufferResizes(agent_name.c_str(), state_name.c_str(), buffLen, force);
    }
    /**
     * Main render mutex used to prevent race conditions rendering agents
     */
    void lockMutex();
    void releaseMutex();
    /**
     * Dynamic line mutex ensures graphs are fully updated before synced to render
     */
    void lockDynamicLinesMutex();
    void releaseDynamicLinesMutex();
    void updateAgentStateBuffer(const std::string &agent_name, const std::string &state_name, const unsigned int buffLen,
        const std::map<TexBufferConfig::Function, TexBufferConfig>& core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& tex_buffers) {
        updateAgentStateBuffer(agent_name.c_str(), state_name.c_str(), buffLen, core_tex_buffers, tex_buffers);
    }
    void updateDynamicLine(const std::string &graph_name) {
        updateDynamicLine(graph_name.c_str());
    }
    /**
     * Provide an environment property, so it can be displayed
     */
    void registerEnvironmentProperty(const std::string& property_name, void* ptr, std::type_index type, unsigned int elements, bool is_const);
    /**
     * Update the UI step counter
     * @note When this value is first set non-0, the visualiser assumes sim has begun executing
     */
    void setStepCount(const unsigned int stepCount);
    /**
     * Provide the random seed, so it can be displayed in the debug menu
     */
    void setRandomSeed(uint64_t randomSeed);
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
    /**
     * Set true when the vis has resized the first agent's buffers
     */
    bool buffersReady() const;
    /**
     * Set true when the vis has had a chance to pause if beginPaused is enabled
     */
    bool isReady() const;

 private:
    void addAgentState(const char *agent_name, const char *state_name, const AgentStateConfig &vc,
        const std::map<TexBufferConfig::Function, TexBufferConfig>& core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& tex_buffersz);
    void requestBufferResizes(const char *agent_name, const char *state_name, const unsigned int buffLen, bool force);
    void updateAgentStateBuffer(const char *agent_name, const char *state_name, const unsigned int buffLen,
        const std::map<TexBufferConfig::Function, TexBufferConfig>& core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& tex_buffers);
    void updateDynamicLine(const char* graph_name);

    Visualiser *vis = nullptr;
    LockHolder *lock = nullptr;
    LockHolder *dynamic_lines_lock = nullptr;
    /**
     * If non-0, limits the number of simulation steps per second, by blocking the return of releaseMutex
     * Blocking whilst the mutex was held, would block visualisation updates as that also uses the mutex
     * The actual value is the number of ms per step
     */
    unsigned int step_ms;
};


}  // namespace visualiser
}  // namespace flamegpu

#endif  // INCLUDE_FLAMEGPU_VISUALISER_FLAMEGPU_VISUALISATION_H_
