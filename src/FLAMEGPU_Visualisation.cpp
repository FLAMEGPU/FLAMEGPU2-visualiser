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

FLAMEGPU_Visualisation::FLAMEGPU_Visualisation(const ModelConfig& modelcfg)
    : vis(new Visualiser(modelcfg))
    // int division is fine, sleeping less is better than sleeping more, system interrupts already mean sleep might be longer
    , step_ms(modelcfg.stepsPerSecond ? 1000 / modelcfg.stepsPerSecond : 0) { }
FLAMEGPU_Visualisation::~FLAMEGPU_Visualisation() {
    if (vis)
        delete vis;
    if (lock)
        delete lock;
}
void FLAMEGPU_Visualisation::addAgentState(const char *agent_name, const char *state_name, const AgentStateConfig &vc,
    const std::map<TexBufferConfig::Function, TexBufferConfig>& core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& tex_buffers) {
    vis->addAgentState(agent_name, state_name, vc, core_tex_buffers, tex_buffers);
}
void FLAMEGPU_Visualisation::requestBufferResizes(const char *agent_name, const char *state_name, const unsigned int buffLen) {
    vis->requestBufferResizes(agent_name, state_name, buffLen);
}
void FLAMEGPU_Visualisation::updateAgentStateBuffer(const char *agent_name, const char *state_name, const unsigned int buffLen,
    const std::map<TexBufferConfig::Function, TexBufferConfig>& core_tex_buffers, const std::multimap<TexBufferConfig::Function, CustomTexBufferConfig>& tex_buffers) {
    vis->updateAgentStateBuffer(agent_name, state_name, buffLen, core_tex_buffers, tex_buffers);
}
void FLAMEGPU_Visualisation::setStepCount(const unsigned int stepCount) {
  vis->setStepCount(stepCount);
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
    // Block the return to hit the desired steps per second
    if (step_ms) {
        static unsigned int last_ticks;
        const unsigned int new_ticks = SDL_GetTicks();
        const int sleep_time = step_ms - (new_ticks - last_ticks);
        if (sleep_time > 0) {
            SDL_Delay(sleep_time);
            last_ticks += step_ms;
        } else if (last_ticks < new_ticks) {
            // Edge case where a step takes longer than step_ms
            last_ticks = new_ticks;
        }
    }
}
