#pragma once

#include <Arduino.h>

#ifndef RSDECK_PERF_TRACE
#define RSDECK_PERF_TRACE 1
#endif

#ifndef RSDECK_PERF_WRITE_TRACE_MS
#define RSDECK_PERF_WRITE_TRACE_MS 20UL
#endif

#ifndef RSDECK_PERF_MSG_TRACE_MS
#define RSDECK_PERF_MSG_TRACE_MS 25UL
#endif

#ifndef RSDECK_PERF_UI_TRACE_MS
#define RSDECK_PERF_UI_TRACE_MS 16UL
#endif

#ifndef RSDECK_PERF_PERSIST_TRACE_MS
#define RSDECK_PERF_PERSIST_TRACE_MS 25UL
#endif

namespace PerfTrace {

inline unsigned long nowMs() {
    return millis();
}

inline unsigned long elapsedMs(unsigned long startMs) {
    return millis() - startMs;
}

inline bool shouldLog(unsigned long durationMs, unsigned long thresholdMs) {
#if RSDECK_PERF_TRACE
    return durationMs >= thresholdMs;
#else
    (void)durationMs;
    (void)thresholdMs;
    return false;
#endif
}

inline void write(const char* backend, const char* op, const char* path,
                  size_t bytes, unsigned long startMs, bool ok,
                  unsigned long thresholdMs = RSDECK_PERF_WRITE_TRACE_MS) {
#if RSDECK_PERF_TRACE
    const unsigned long durationMs = elapsedMs(startMs);
    if (!ok || durationMs >= thresholdMs) {
        Serial.printf("[PERF] WRITE backend=%s op=%s path=%s bytes=%u ok=%s in %lums\n",
                      backend ? backend : "?",
                      op ? op : "?",
                      path ? path : "?",
                      (unsigned)bytes,
                      ok ? "yes" : "no",
                      durationMs);
    }
#else
    (void)backend;
    (void)op;
    (void)path;
    (void)bytes;
    (void)startMs;
    (void)ok;
    (void)thresholdMs;
#endif
}

}  // namespace PerfTrace
