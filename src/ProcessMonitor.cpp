#include "ProcessMonitor.h"
#include <tlhelp32.h>
#include <iostream>

std::vector<ProcessInfo> ProcessMonitor::ScanProcesses() {
    std::vector<ProcessInfo> processes;
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return processes;
    }

    cProcesses = cbNeeded / sizeof(DWORD);

    for (i = 0; i < cProcesses; i++) {
        if (aProcesses[i] != 0) {
            DWORD currentPid = aProcesses[i];
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, currentPid);

            if (hProcess != NULL) {
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    ProcessInfo info;
                    info.pid = currentPid;
                    info.name = GetProcessName(hProcess);
                    
                    wchar_t fullPath[MAX_PATH];
                    DWORD size = MAX_PATH;
                    if (QueryFullProcessImageNameW(hProcess, 0, fullPath, &size)) {
                        info.path = std::wstring(fullPath);
                    } else {
                        info.path = L"";
                    }

                    info.memoryMB = (double)pmc.WorkingSetSize / (1024.0 * 1024.0);
                    info.threadCount = GetThreadCount(currentPid);
                    info.cpuPercent = this->CalculateProcessCpuLoad(currentPid, hProcess);
                    
                    DWORD hCount = 0;
                    GetProcessHandleCount(hProcess, &hCount);
                    info.handleCount = hCount;
                    
                    info.category = BehaviorCategory::UNKNOWN;
                    info.lastOptimizedTimestamp = std::chrono::steady_clock::time_point::min();
                    
                    processes.push_back(info);
                }
                CloseHandle(hProcess);
            }
        }
    }

    return processes;
}

std::wstring ProcessMonitor::GetProcessName(HANDLE hProcess) {
    wchar_t szProcessName[MAX_PATH] = L"<unknown>";
    HMODULE hMod;
    DWORD cbNeeded;

    if (K32EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
        K32GetModuleBaseNameW(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(wchar_t));
    }
    return std::wstring(szProcessName);
}

int ProcessMonitor::GetThreadCount(DWORD pid) {
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE) return 0;

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    int count = 0;
    if (Thread32First(hThreadSnap, &te32)) {
        do {
            if (te32.th32OwnerProcessID == pid) {
                count++;
            }
        } while (Thread32Next(hThreadSnap, &te32));
    }
    CloseHandle(hThreadSnap);
    return count;
}

double ProcessMonitor::CalculateProcessCpuLoad(DWORD pid, HANDLE hProcess) {
    FILETIME ftCreation, ftExit, ftKernel, ftUser;
    FILETIME ftSystemIdle, ftSystemKernel, ftSystemUser;

    if (GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser) && GetSystemTimes(&ftSystemIdle, &ftSystemKernel, &ftSystemUser)) {
        ULARGE_INTEGER kernel, user;
        kernel.LowPart = ftKernel.dwLowDateTime; kernel.HighPart = ftKernel.dwHighDateTime;
        user.LowPart = ftUser.dwLowDateTime; user.HighPart = ftUser.dwHighDateTime;

        ULARGE_INTEGER sysKernel, sysUser;
        sysKernel.LowPart = ftSystemKernel.dwLowDateTime; sysKernel.HighPart = ftSystemKernel.dwHighDateTime;
        sysUser.LowPart = ftSystemUser.dwLowDateTime; sysUser.HighPart = ftSystemUser.dwHighDateTime;

        ULONGLONG currentProcTime = kernel.QuadPart + user.QuadPart;
        ULONGLONG currentSystemTime = sysKernel.QuadPart + sysUser.QuadPart;

        if (timeCache.find(pid) != timeCache.end()) {
            ULARGE_INTEGER prevKernel, prevUser;
            prevKernel.LowPart = timeCache[pid].kernelTime.dwLowDateTime;
            prevKernel.HighPart = timeCache[pid].kernelTime.dwHighDateTime;
            prevUser.LowPart = timeCache[pid].userTime.dwLowDateTime;
            prevUser.HighPart = timeCache[pid].userTime.dwHighDateTime;

            ULONGLONG prevProcTime = prevKernel.QuadPart + prevUser.QuadPart;
            ULONGLONG prevSystemTime = timeCache[pid].lastSystemTime;

            ULONGLONG procDelta = currentProcTime - prevProcTime;
            ULONGLONG systemDelta = currentSystemTime - prevSystemTime;

            double usage = 0.0;
            if (systemDelta > 0) {
                usage = (double)(procDelta * 100.0) / systemDelta;
            }
            
            timeCache[pid].kernelTime = ftKernel;
            timeCache[pid].userTime = ftUser;
            timeCache[pid].lastSystemTime = currentSystemTime;
            return usage > 100.0 ? 100.0 : usage;
        } else {
            timeCache[pid] = { ftKernel, ftUser, currentSystemTime };
        }
    }
    return 0.0;
}
