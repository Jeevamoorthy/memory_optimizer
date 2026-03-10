#include "OptimizationEngine.h"
#include <vector>
#include <sstream>
#include <algorithm>
#include <windows.h>
#include <psapi.h>
#include "Logger.h"
#include "AIDecisionEngine.h"

std::vector<OptimizationTask> OptimizationEngine::ProcessOptimizations(std::vector<ProcessInfo>& processes, bool isHighLoad) {
    std::vector<OptimizationTask> tasks;
    auto now = std::chrono::steady_clock::now();

    // 1. Grouping for browsers
    struct GroupInfo {
        double totalMem = 0;
        double totalCpu = 0;
        int count = 0;
    };
    std::map<std::wstring, GroupInfo> browserGroups;
    std::vector<std::wstring> browserNames = { L"chrome.exe", L"brave.exe", L"msedge.exe", L"msedgewebview2.exe" };

    for (auto& proc : processes) {
        bool isBrowser = false;
        for (const auto& bName : browserNames) {
            if (proc.name == bName) {
                isBrowser = true;
                break;
            }
        }
        if (isBrowser) {
            browserGroups[proc.name].totalMem += proc.memoryMB;
            browserGroups[proc.name].totalCpu += proc.cpuPercent;
            browserGroups[proc.name].count++;
        }
    }

    // Trigger group optimization if aggregate memory > 2000MB (Master Prompt)
    for (auto const& [name, info] : browserGroups) {
        if (info.totalMem > 2000.0) {
            std::wstringstream gss;
            gss << L"HEAVY GROUP DETECTED: " << name << L" (" << (int)info.totalMem << L"MB). Applying aggregate optimization.";
            Logger::LogAction(L"SYSTEM", 0, gss.str());
        }
    }

    DWORD selfPid = GetCurrentProcessId();

    for (auto& proc : processes) {
        // SAFETY LAYER: Never optimize self, CRITICAL or processes in Windows directory
        if (proc.pid == selfPid || 
            proc.category == BehaviorCategory::CRITICAL || 
            proc.path.find(L"C:\\Windows") == 0 || 
            proc.path.find(L"c:\\windows") == 0) {
            continue;
        }

        // Never optimize active processes (Master Prompt: CPU > 5%)
        if (proc.cpuPercent > 5.0) continue;

        // Check cooldown (30 seconds)
        if (lastOptimizationTime.count(proc.pid)) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastOptimizationTime[proc.pid]).count();
            if (elapsed < 30) continue;
        }

        // AI DECISION ENGINE
        double groupMem = browserGroups.count(proc.name) ? browserGroups[proc.name].totalMem : 0.0;
        if (!AIDecisionEngine::ShouldOptimize(proc, groupMem)) continue;

        OptimizationTask task;
        task.pid = proc.pid;
        task.name = proc.name;
        task.apply = false;
        std::wstringstream ss;

        // A. Process Priority Optimization
        if (proc.category == BehaviorCategory::BACKGROUND || proc.category == BehaviorCategory::TARGET_APP) {
            SetPriority(proc.pid, BELOW_NORMAL_PRIORITY_CLASS);
            ss << L"Priority lowered";
            task.apply = true;
        }

        // B. CPU Affinity Optimization
        if (isHighLoad && proc.category == BehaviorCategory::BACKGROUND) {
            SetAffinity(proc.pid, true);
            if (task.apply) ss << L" + ";
            ss << L"Affinity reduced";
            task.apply = true;
        }

        // C. Working Set Memory Trimming (60s cooldown)
        bool canTrim = true;
        if (lastTrimTime.count(proc.pid)) {
            auto elapsedTrim = std::chrono::duration_cast<std::chrono::seconds>(now - lastTrimTime[proc.pid]).count();
            if (elapsedTrim < 60) canTrim = false;
        }

        if (canTrim && proc.memoryMB > 300.0) {
            double saved = TrimWorkingSet(proc.pid);
            lastTrimTime[proc.pid] = now;
            if (task.apply) ss << L" + ";
            ss << L"Memory trimmed";
            task.savedMB = saved;
            task.apply = true;
        }

        // D. Memory Priority Reduction (New Action)
        SetMemoryPriority(proc.pid, 1); // Very Low Priority
        if (task.apply) ss << L" + ";
        ss << L"MemPriority set to 1";
        task.apply = true;

        if (task.apply) {
            task.actions = ss.str();
            lastOptimizationTime[proc.pid] = now;
            proc.lastOptimizedTimestamp = now;
            tasks.push_back(task);
            Logger::LogAction(proc.name, proc.pid, task.actions);
        }
    }
    return tasks;
}

double OptimizationEngine::TrimWorkingSet(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_SET_QUOTA | PROCESS_QUERY_INFORMATION, FALSE, pid);
    double savedMB = 0.0;
    if (hProcess) {
        PROCESS_MEMORY_COUNTERS pmcBefore;
        if (GetProcessMemoryInfo(hProcess, &pmcBefore, sizeof(pmcBefore))) {
            if (EmptyWorkingSet(hProcess)) {
                PROCESS_MEMORY_COUNTERS pmcAfter;
                if (GetProcessMemoryInfo(hProcess, &pmcAfter, sizeof(pmcAfter))) {
                    savedMB = (double)(pmcBefore.WorkingSetSize - pmcAfter.WorkingSetSize) / (1024.0 * 1024.0);
                }
            }
        }
        CloseHandle(hProcess);
    }
    return savedMB > 0 ? savedMB : 0.0;
}

void OptimizationEngine::SetAffinity(DWORD pid, bool halfCores) {
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess) {
        DWORD_PTR processAffinityMask;
        DWORD_PTR systemAffinityMask;
        if (GetProcessAffinityMask(hProcess, &processAffinityMask, &systemAffinityMask)) {
            if (halfCores) {
                // Set to half of system cores
                DWORD_PTR newMask = 0;
                int count = 0;
                for (int i = 0; i < sizeof(DWORD_PTR) * 8; i++) {
                    if (systemAffinityMask & ((DWORD_PTR)1 << i)) {
                        if (count % 2 == 0) {
                            newMask |= ((DWORD_PTR)1 << i);
                        }
                        count++;
                    }
                }
                SetProcessAffinityMask(hProcess, newMask);
            }
        }
        CloseHandle(hProcess);
    }
}

void OptimizationEngine::SetPriority(DWORD pid, DWORD priorityClass) {
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (hProcess) {
        SetPriorityClass(hProcess, priorityClass);
        CloseHandle(hProcess);
    }
}

void OptimizationEngine::SetMemoryPriority(DWORD pid, ULONG priority) {
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
    if (hProcess) {
        MEMORY_PRIORITY_INFORMATION mpi = { 0 };
        mpi.MemoryPriority = priority;
        SetProcessInformation(hProcess, ProcessMemoryPriority, &mpi, sizeof(mpi));
        CloseHandle(hProcess);
    }
}
