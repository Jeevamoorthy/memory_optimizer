#pragma once
#include "BehaviorClassifier.h"
#include <vector>
#include <map>
#include <chrono>

enum class OptimizationAction {
    None,
    LowerPriority
};

struct OptimizationTask {
    DWORD pid;
    std::wstring name;
    std::wstring actions; // Descriptive actions like "Priority lowered + Memory trimmed"
    double savedMB = 0.0;
    bool apply;
};

class OptimizationEngine {
public:
    std::vector<OptimizationTask> ProcessOptimizations(std::vector<ProcessInfo>& processes, bool isHighLoad);
private:
    std::map<DWORD, std::chrono::steady_clock::time_point> lastOptimizationTime;
    std::map<DWORD, std::chrono::steady_clock::time_point> lastTrimTime;
    double TrimWorkingSet(DWORD pid);
    void SetAffinity(DWORD pid, bool halfCores);
    void SetPriority(DWORD pid, DWORD priorityClass);
    void SetMemoryPriority(DWORD pid, ULONG priority);
};
