#pragma once
#include <string>
#include <iostream>
#include <windows.h>

class Logger {
public:
    static void Log(const std::wstring& message) {
        std::wcout << L"[INFO] " << message << std::endl;
    }

    static void LogAction(const std::wstring& procName, DWORD pid, const std::wstring& action) {
        std::wcout << L"--------------------------------" << std::endl;
        std::wcout << L"Optimizing process: " << procName << L" (PID: " << pid << L")" << std::endl;
        std::wcout << L"Action: " << action << std::endl;
    }

    static void LogStats(int monitored, int optimized, double memSavedMB) {
        std::wcout << L"================================" << std::endl;
        std::wcout << L"Processes monitored: " << monitored << std::endl;
        std::wcout << L"Optimized processes: " << optimized << std::endl;
        std::wcout << L"Memory trimmed: " << memSavedMB << L" MB" << std::endl;
        std::wcout << L"================================" << std::endl;
    }
};
