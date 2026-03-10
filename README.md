# UROE: Universal Resource Optimization Engine (v3.1)

![Project Status](https://img.shields.io/badge/status-active-brightgreen)
![C++](https://img.shields.io/badge/C++-17-blue)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey)

**UROE** is a high-performance, AI-driven Windows system resource optimizer designed to intelligently manage background workloads. It monitors system pressure and applies safe, reversible optimizations to reduce CPU and memory usage of heavy applications (Browsers, IDEs, etc.) without affecting system stability.

## 🚀 Key Features

- **🧠 AI Decision Engine**: Uses a native Logistic Regression model (Sigmoid activation) to predict optimization necessity based on CPU, memory, thread counts, and handle usage.
- **📊 Real-time Dashboard**: Integrated 5-second monitoring loop with a professional console dashboard displaying PID, category, and real-time resource usage.
- **🌐 Browser Group Optimization**: Detects browser clusters (Brave, Chrome, Edge) and applies aggregate optimizations when group memory exceeds 2GB.
- **🛡️ Advanced Safety Layer**: 
    - Excludes critical Windows processes and system paths.
    - Prevents optimization of the optimizer itself.
    - No optimizations for active processes (>5% CPU usage).
- **🔋 Memory Management**:
    - **Memory Trim**: Working set reduction using `EmptyWorkingSet`.
    - **Memory Priority**: Sets background process memory priority to Level 1 (Very Low).
    - **CPU Affinity**: Dynamically restricts background tasks to half of available cores.

## 🏗️ System Architecture & Layers

UROE operates through a multi-layered pipeline, ensuring each optimization is data-driven and safe.

| Layer | Component | Responsibility | Significance / Importance |
| :--- | :--- | :--- | :--- |
| **1. Observation** | `ProcessMonitor` | Scans all active PIDs; collects CPU, RAM (Working/Private Set), Thread, and Handle counts. | Provides the raw telemetry required for informed decision-making. |
| **2. Context** | `SystemMetrics` | Monitors global system-wide CPU and RAM saturation. | Determines the "Mode" (Monitoring vs. Active Optimization) based on system pressure. |
| **3. Logic** | `BehaviorClassifier` | Assigns roles like `CRITICAL`, `BACKGROUND`, or `ACTIVE` based on heuristics and paths. | **Safety Gate**: Ensures Windows core services and active user apps are never throttled. |
| **4. Intelligence** | `AIDecisionEngine`| Native Logistic Regression model using Sigmoid activation. | Calculates the **Optimization Probability**. Actions only fire if `p > 0.65`. |
| **5. Action** | `OptimizationEngine`| Modifies Process Priority, Core Affinity, and Working Sets. | Implements physical resource reclamation (Memory Priority Level 1). |
| **6. Visibility** | `Main Dashboard` | Renders a professional real-time console UI with "Net Benefit" logging. | Provides transparency into how much memory the tool is actually saving. |

## 📈 Net Benefit Analysis

UROE v3.1 features a transparent "Net Benefit" display. It calculates its own RAM footprint and subtracts it from the total cumulative memory saved, showing the real-world efficiency gain for the system.

## 🛠️ How to Build

### Requirements
- C++17 compliant compiler (GCC/MinGW recommended)
- Windows SDK (linking with `psapi.lib`)

### Compilation (MinGW/GCC)
```powershell
g++ -std=c++17 -DUNICODE -D_UNICODE src/*.cpp -o UROE.exe -lpsapi
```
<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/7d174855-007a-44d3-8e23-8bafa956386d" />


## ⚖️ Importance
Modern applications (especially Electron-based tools and browsers) often claim more resources than they actively use. UROE acts as an intelligent layer between these apps and the OS, ensuring that background tasks do not degrade the experience of the active user, effectively acting as an automated system resource scheduler.

---
Developed by Jeevamoorthy
