#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <psapi.h>
#include <map>

#include <chrono>

enum class BehaviorCategory {
    CRITICAL,
    ACTIVE,
    BACKGROUND,
    TARGET_APP,
    UNKNOWN
};

struct ProcessInfo {
    DWORD pid;
    std::wstring name;
    std::wstring path;
    double memoryMB;
    double cpuPercent;
    int threadCount;
    DWORD handleCount;
    BehaviorCategory category;
    std::chrono::steady_clock::time_point lastOptimizedTimestamp;
};

struct ProcessTimeRecord {
    FILETIME kernelTime;
    FILETIME userTime;
    ULONGLONG lastSystemTime;
};

class ProcessMonitor {
public:
    std::vector<ProcessInfo> ScanProcesses();
private:
    std::wstring GetProcessName(HANDLE hProcess);
    int GetThreadCount(DWORD pid);
    double CalculateProcessCpuLoad(DWORD pid, HANDLE hProcess);
    std::map<DWORD, ProcessTimeRecord> timeCache;
};
