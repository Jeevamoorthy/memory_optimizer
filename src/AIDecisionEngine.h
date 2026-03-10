#pragma once
#include "ProcessMonitor.h"
#include <cmath>

class AIDecisionEngine {
public:
    static bool ShouldOptimize(const ProcessInfo& proc, double groupPressureMB);

private:
    static double Sigmoid(double x);
    
    // Model weights (tuned for conservative optimization)
    static constexpr double wCPU = 0.8;
    static constexpr double wMem = 0.005;
    static constexpr double wThreads = 0.15;
    static constexpr double wHandles = 0.05;
    static constexpr double wGroup = 0.002;
    static constexpr double bias = -4.5; // Bias towards NOT optimizing unless metrics are significant
};
