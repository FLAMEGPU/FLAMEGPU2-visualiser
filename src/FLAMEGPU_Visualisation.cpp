#include "FLAMEGPU_Visualisation.h"
#include "Visualiser.h"

#include <mutex>

class LockHolder {
 public:
    explicit LockHolder(std::mutex &mtx)
        : guard(mtx) { }
 private:
    std::lock_guard<std::mutex> guard;
};

FLAMEGPU_Visualisation::FLAMEGPU_Visualisation(const unsigned int width, const unsigned int height)
    : vis(new Visualiser(width, height)) { }
FLAMEGPU_Visualisation::~FLAMEGPU_Visualisation() {
    if (vis)
        delete vis;
    if (lock)
        delete lock;
}
void FLAMEGPU_Visualisation::addAgentState(const char *agent_name, const char *state_name, const AgentStateConfig &vc) {
    vis->addAgentState(agent_name, state_name, vc);
}
void FLAMEGPU_Visualisation::requestBufferResizes(const char *agent_name, const char *state_name, const unsigned int buffLen) {
    vis->requestBufferResizes(agent_name, state_name, buffLen);
}
void FLAMEGPU_Visualisation::updateAgentStateBuffer(const char *agent_name, const char *state_name, const unsigned int buffLen, float *d_x, float *d_y, float *d_z) {
    vis->updateAgentStateBuffer(agent_name, state_name, buffLen, d_x, d_y, d_z);
}

void FLAMEGPU_Visualisation::start() {
    vis->start();
}
void FLAMEGPU_Visualisation::join() {
    vis->join();
}
void FLAMEGPU_Visualisation::stop() {
    vis->stop();
}

bool FLAMEGPU_Visualisation::isRunning() const {
    return vis->isRunning();
}


void FLAMEGPU_Visualisation::lockMutex() {
    lock = new LockHolder(vis->getRenderBufferMutex());
}
void FLAMEGPU_Visualisation::releaseMutex() {
    if (lock) {
        delete lock;
        lock = nullptr;
    }
}
