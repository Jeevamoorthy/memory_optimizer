#include "BehaviorClassifier.h"
#include <algorithm>

void BehaviorClassifier::Classify(std::vector<ProcessInfo>& processes) {
    for (auto& proc : processes) {
        bool isTarget = IsTargetApp(proc.name);
        
        // CRITICAL Rule: Processes inside Windows directory
        bool isCriticalPath = (proc.path.find(L"C:\\Windows") == 0 || proc.path.find(L"c:\\windows") == 0);
        
        std::vector<std::wstring> criticalNames = {
            L"dwm.exe", L"csrss.exe", L"lsass.exe", 
            L"services.exe", L"svchost.exe", L"winlogon.exe", L"smss.exe",
            L"explorer.exe", L"taskhostw.exe", L"sihost.exe"
        };

        bool isCriticalName = false;
        for (const auto& name : criticalNames) {
            if (proc.name == name) {
                isCriticalName = true;
                break;
            }
        }

        if (isCriticalPath || isCriticalName) {
            proc.category = BehaviorCategory::CRITICAL;
        } else if (isTarget) {
            proc.category = BehaviorCategory::TARGET_APP;
        } else if (proc.cpuPercent > 5.0) {
            proc.category = BehaviorCategory::ACTIVE;
        } else if (proc.cpuPercent < 1.0 && proc.memoryMB > 200.0) {
            proc.category = BehaviorCategory::BACKGROUND;
        } else {
            proc.category = BehaviorCategory::UNKNOWN;
        }
    }
}

bool BehaviorClassifier::IsTargetApp(const std::wstring& name) {
    std::vector<std::wstring> targets = { 
        L"chrome.exe", L"brave.exe", L"Code.exe", 
        L"studio64.exe", L"flutter.exe", L"dart.exe",
        L"electron.exe", L"msedgewebview2.exe"
    };
    
    for (const auto& target : targets) {
        if (name == target) return true;
    }
    return false;
}
