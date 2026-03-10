#include "SystemMetrics.h"

SystemMetrics::SystemMetrics() : prevKernel(0), prevUser(0), prevIdle(0) {
    FILETIME idle, kernel, user;
    if (GetSystemTimes(&idle, &kernel, &user)) {
        ULARGE_INTEGER i, k, u;
        i.LowPart = idle.dwLowDateTime; i.HighPart = idle.dwHighDateTime;
        k.LowPart = kernel.dwLowDateTime; k.HighPart = kernel.dwHighDateTime;
        u.LowPart = user.dwLowDateTime; u.HighPart = user.dwHighDateTime;
        prevIdle = i.QuadPart;
        prevKernel = k.QuadPart;
        prevUser = u.QuadPart;
    }
}

SystemData SystemMetrics::GetCurrentLoad() {
    SystemData data;
    data.cpu = GetCpuUsage();
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);
    data.ram = (double)mem.dwMemoryLoad;
    return data;
}

bool SystemMetrics::IsHighLoad() {
    SystemData d = GetCurrentLoad();
    return (d.cpu > 5.0 || d.ram > 50.0);
}

double SystemMetrics::GetCpuUsage() {
    FILETIME idle, kernel, user;
    if (GetSystemTimes(&idle, &kernel, &user)) {
        ULARGE_INTEGER i, k, u;
        i.LowPart = idle.dwLowDateTime; i.HighPart = idle.dwHighDateTime;
        k.LowPart = kernel.dwLowDateTime; k.HighPart = kernel.dwHighDateTime;
        u.LowPart = user.dwLowDateTime; u.HighPart = user.dwHighDateTime;

        ULONGLONG curI = i.QuadPart;
        ULONGLONG curK = k.QuadPart;
        ULONGLONG curU = u.QuadPart;

        ULONGLONG dI = curI - prevIdle;
        ULONGLONG dK = curK - prevKernel;
        ULONGLONG dU = curU - prevUser;

        ULONGLONG total = dK + dU;
        ULONGLONG busy = total - dI;

        prevIdle = curI;
        prevKernel = curK;
        prevUser = curU;

        if (total == 0) return 0.0;
        double usage = (double)(busy * 100.0) / total;
        return (usage < 0.0) ? 0.0 : (usage > 100.0 ? 100.0 : usage);
    }
    return 0.0;
}
