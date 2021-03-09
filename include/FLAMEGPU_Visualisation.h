#ifndef INCLUDE_FLAMEGPU_VISUALISATION_H_
#define INCLUDE_FLAMEGPU_VISUALISATION_H_

#include <string>

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
    void addAgentState(const std::string &agent_name, const std::string &state_name, const AgentStateConfig &vc, bool has_x, bool has_y, bool has_z) {
        addAgentState(agent_name.c_str(), state_name.c_str(), vc, has_x, has_y, has_z);
    }
    void requestBufferResizes(const std::string &agent_name, const std::string &state_name, const unsigned int buffLen) {
        requestBufferResizes(agent_name.c_str(), state_name.c_str(), buffLen);
    }
    void lockMutex();
    void releaseMutex();
    void updateAgentStateBuffer(const std::string &agent_name, const std::string &state_name, const unsigned int buffLen, float *d_x, float *d_y, float *d_z) {
        updateAgentStateBuffer(agent_name.c_str(), state_name.c_str(), buffLen, d_x, d_y, d_z);
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
    void addAgentState(const char *agent_name, const char *state_name, const AgentStateConfig &vc, bool has_x = true, bool has_y = true, bool has_z = true);
    void requestBufferResizes(const char *agent_name, const char *state_name, const unsigned int buffLen);
    void updateAgentStateBuffer(const char *agent_name, const char *state_name, const unsigned int buffLen, float *d_x, float *d_y, float *d_z);
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
