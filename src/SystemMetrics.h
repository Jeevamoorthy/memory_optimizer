#ifndef SYSTEM_METRICS_H
#define SYSTEM_METRICS_H

#include <windows.h>

struct SystemData {
    double cpu;
    double ram;
};

class SystemMetrics {
public:
    SystemMetrics();
    SystemData GetCurrentLoad();
    bool IsHighLoad();
private:
    double GetCpuUsage();
    ULONGLONG prevKernel;
    ULONGLONG prevUser;
    ULONGLONG prevIdle;
};

#endif
