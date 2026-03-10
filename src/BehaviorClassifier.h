#pragma once
#include "ProcessMonitor.h"
#include <vector>

class BehaviorClassifier {
public:
    void Classify(std::vector<ProcessInfo>& processes);
private:
    bool IsTargetApp(const std::wstring& name);
};
