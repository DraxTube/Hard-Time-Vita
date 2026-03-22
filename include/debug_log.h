#pragma once
// =============================================================================
//  debug_log.h  –  Simple file logger for debugging crashes on PS Vita
//  Writes to ux0:data/HardTime/debug.log (flushed after every write)
// =============================================================================
#include <cstdio>
#include <cstdarg>

inline FILE* g_debugLog = nullptr;

inline void DebugLogInit() {
    g_debugLog = fopen("ux0:data/HardTime/debug.log", "w");
    if (g_debugLog) {
        fprintf(g_debugLog, "=== Hard Time Vita Debug Log ===\n");
        fflush(g_debugLog);
    }
}

inline void DebugLog(const char* fmt, ...) {
    if (!g_debugLog) return;
    va_list args;
    va_start(args, fmt);
    vfprintf(g_debugLog, fmt, args);
    va_end(args);
    fprintf(g_debugLog, "\n");
    fflush(g_debugLog);   // flush every line so we see last msg before crash
}

inline void DebugLogClose() {
    if (g_debugLog) { fclose(g_debugLog); g_debugLog = nullptr; }
}
