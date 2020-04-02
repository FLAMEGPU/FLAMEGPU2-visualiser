#ifndef INCLUDE_FLAMEGPU_VISUALISATION_H_
#define INCLUDE_FLAMEGPU_VISUALISATION_H_

#include <string>

struct AgentStateConfig;
class Visualiser;
class LockHolder;

/**
 * This class forms the interface between FLAMEGPU2 and FLAMEGPU2_Visualiser
 * It is important that it remains raw C (or any std is inline only)
 */
class FLAMEGPU_Visualisation {
 public:
    explicit FLAMEGPU_Visualisation(const unsigned int window_width = 1280, const unsigned int window_height = 720);
    ~FLAMEGPU_Visualisation();
    void addAgentState(const std::string &agent_name, const std::string &state_name, const AgentStateConfig &vc) {
        addAgentState(agent_name.c_str(), state_name.c_str(), vc);
    }
    void requestBufferResizes(const std::string &agent_name, const std::string &state_name, const unsigned int buffLen) {
        requestBufferResizes(agent_name.c_str(), state_name.c_str(), buffLen);
    }
    void lockMutex();
    void releaseMutex();
    void updateAgentStateBuffer(const std::string &agent_name, const std::string &state_name, const unsigned int buffLen, float *d_x, float *d_y, float *d_z) {
        updateAgentStateBuffer(agent_name.c_str(), state_name.c_str(), buffLen, d_x, d_y, d_z);
    }
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
    void addAgentState(const char *agent_name, const char *state_name, const AgentStateConfig &vc);
    void requestBufferResizes(const char *agent_name, const char *state_name, const unsigned int buffLen);
    void updateAgentStateBuffer(const char *agent_name, const char *state_name, const unsigned int buffLen, float *d_x, float *d_y, float *d_z);
    Visualiser *vis = nullptr;
    LockHolder *lock = nullptr;
};

#endif  // INCLUDE_FLAMEGPU_VISUALISATION_H_
