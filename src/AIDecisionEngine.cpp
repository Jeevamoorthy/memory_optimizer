#include "AIDecisionEngine.h"

bool AIDecisionEngine::ShouldOptimize(const ProcessInfo& proc, double groupPressureMB) {
    // Features: CPU %, Memory MB, Thread Count, Handle Count, Group Pressure
    double z = (proc.cpuPercent * wCPU) + 
               (proc.memoryMB * wMem) + 
               (proc.threadCount * wThreads) + 
               (proc.handleCount * wHandles) + 
               (groupPressureMB * wGroup) + 
               bias;
               
    double probability = Sigmoid(z);
    
    // Threshold from master prompt: 0.65
    return probability > 0.65;
}

double AIDecisionEngine::Sigmoid(double x) {
    return 1.0 / (1.0 + std::exp(-x));
}
