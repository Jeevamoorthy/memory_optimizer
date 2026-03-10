#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <vector>
#include <windows.h>
#include "ProcessMonitor.h"
#include "BehaviorClassifier.h"
#include "OptimizationEngine.h"
#include "SystemMetrics.h"
#include "Logger.h"

void PrintHeader(const SystemData& load, bool isOptimizing) {
    system("cls");
    std::wcout << L"================================================" << std::endl;
    std::wcout << L"   UNIVERSAL RESOURCE OPTIMIZATION ENGINE (UROE)" << std::endl;
    std::wcout << L"           AI-Driven Optimizer v3.0" << std::endl;
    std::wcout << L"================================================" << std::endl;
    std::wcout << L"System Load: CPU " << std::fixed << std::setprecision(1) << load.cpu << L"%, "
               << L"RAM " << load.ram << L"%" << std::endl;
    std::wcout << L"Optimization Mode: " << (isOptimizing ? L"ACTIVE_OPTIMIZATION" : L"MONITORING") << std::endl;
    std::wcout << L"------------------------------------------------" << std::endl;
}

std::wstring GetCategoryStr(BehaviorCategory cat) {
    switch (cat) {
        case BehaviorCategory::CRITICAL: return L"CRITICAL";
        case BehaviorCategory::ACTIVE: return L"ACTIVE";
        case BehaviorCategory::BACKGROUND: return L"BACKGROUND";
        case BehaviorCategory::TARGET_APP: return L"TARGET_APP";
        default: return L"UNKNOWN";
    }
}

int main() {
    SystemMetrics metrics;
    ProcessMonitor monitor;
    BehaviorClassifier classifier;
    OptimizationEngine engine;
    
    double totalSavedMB = 0.0;
    DWORD currentPid = GetCurrentProcessId();

    while (true) {
        SystemData load = metrics.GetCurrentLoad();
        bool isHighLoad = metrics.IsHighLoad();
        
        auto processes = monitor.ScanProcesses();
        classifier.Classify(processes);
        auto tasks = engine.ProcessOptimizations(processes, isHighLoad);

        // Track savings
        for (const auto& task : tasks) {
            totalSavedMB += task.savedMB;
        }

        // Get tool's own RAM
        double toolRamMB = 0.0;
        HANDLE hSelf = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, currentPid);
        if (hSelf) {
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hSelf, &pmc, sizeof(pmc))) {
                toolRamMB = (double)pmc.WorkingSetSize / (1024.0 * 1024.0);
            }
            CloseHandle(hSelf);
        }

        PrintHeader(load, isHighLoad);
        
        std::wcout << std::left << std::setw(8) << L"PID" 
                   << std::setw(15) << L"NAME" 
                   << std::setw(10) << L"CPU"
                   << std::setw(15) << L"MEM" 
                   << std::setw(15) << L"CATEGORY" << std::endl;

        int displayCount = 0;
        for (const auto& proc : processes) {
            if (proc.category != BehaviorCategory::UNKNOWN) {
                if (displayCount < 18) {
                    std::wstringstream cpuSS, memSS;
                    cpuSS << std::fixed << std::setprecision(1) << proc.cpuPercent << L"%";
                    memSS << (int)proc.memoryMB << L"MB";
                    
                    std::wcout << std::left << std::setw(8) << proc.pid 
                               << std::setw(15) << proc.name.substr(0, 14) 
                               << std::setw(10) << cpuSS.str()
                               << std::setw(15) << memSS.str()
                               << std::setw(15) << GetCategoryStr(proc.category) << std::endl;
                    displayCount++;
                }
            }
        }

        std::wcout << L"------------------------------------------------" << std::endl;
        std::wcout << L"Total Saved: " << std::fixed << std::setprecision(1) << totalSavedMB << L" MB"
                   << L" | Tool RAM: " << (int)toolRamMB << L" MB"
                   << L" | Net Benefit: " << (totalSavedMB - toolRamMB) << L" MB" << std::endl;
        std::wcout << L"------------------------------------------------" << std::endl;
        
        if (!tasks.empty()) {
            std::wcout << L"Recent Actions:" << std::endl;
            for (size_t i = 0; i < (tasks.size() > 5 ? 5 : tasks.size()); ++i) {
                std::wcout << L"[OK] " << tasks[i].name << L": " << tasks[i].actions << std::endl;
            }
        } else {
            std::wcout << L"System stable. No optimizations needed." << std::endl;
        }

        std::wcout << L"\nNext scan in 5 seconds..." << std::endl;
        Sleep(5000);
    }

    return 0;
}
