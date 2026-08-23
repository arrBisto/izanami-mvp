#pragma once
#include <string>

namespace CrashGuard {
    bool PreviousSessionCrashed();
    std::string GenerateCrashReport();
    void Initialize();
    void ArmWatchdog(int seconds);
    void DisarmWatchdog();
    void LogEvent(const std::string& event);
    void ShutdownClean();
}
