#pragma once

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#ifdef _WIN32
#include <windows.h>
#else
#include <chrono>
#endif

#include <string>

#include "thirdparty/json/single_include/nlohmann/json.hpp"
using json = nlohmann::json;

class TraceWriter;

enum EventCategory {
  METADATA,
  DEFAULT,
  IO_FS,
  VM_COMPILE_FILE,
  VM_LOAD_OBJECT,
  APPLY_CACHE,
  LPC_CATCH,
  LPC_FUNCTION,
  LPC_EFUN,
};

class Event {
 public:
  Event();

  const unsigned long process_id;
  const unsigned long thread_id;
  double timestamp;
  EventCategory category = EventCategory::DEFAULT;
  const char* phase = "i";
  std::string name = "DEFAULT";
  json args = {};

  inline const char* category_name() {
    switch (this->category) {
      case EventCategory::METADATA:
        return "__metadata";
      case EventCategory::DEFAULT:
        return "DEFAULT";
      case EventCategory::IO_FS:
        return "IO: Filesystem";
      case EventCategory::VM_COMPILE_FILE:
        return "VM: Compile File";
      case EventCategory::VM_LOAD_OBJECT:
        return "VM: Load Object";
      case EventCategory::APPLY_CACHE:
        return "Apply cache lookup";
      case EventCategory::LPC_CATCH:
        return "LPC Catch";
      case EventCategory::LPC_FUNCTION:
        return "LPC Function";
      case EventCategory::LPC_EFUN:
        return "LPC EFUN";
    }
    return "Unknown";
  }
};

class Tracer {
 public:
  static void setThreadName(const std::string& name);
  static void log(const Event& e);
  static void counter(const std::string& name, long n);
  static void counter(const std::string& name, const json& args = {});
  static void logSimpleEvent(const std::string& name, const EventCategory& category);
  static void begin(const std::string& name, const EventCategory& category, json&& args = {});
  static void end(const std::string&, const EventCategory& category);

  static inline void start(const char* file) {
    // If there is already events, flush it out first.
    if (is_enabled) {
      collect();
    }
#ifdef _WIN32
    QueryPerformanceCounter(&basetime);
#else
    basetime = std::chrono::steady_clock::now();
#endif
    filename = file;
    is_enabled = true;
  }

  static inline void stop() { is_enabled = false; }

  static inline bool enabled() { return is_enabled; }

  static inline auto timestamp() {
#ifdef _WIN32
    static LARGE_INTEGER Frequency{};
    if (Frequency.QuadPart == 0) {
      QueryPerformanceFrequency(&Frequency);
    }

    LARGE_INTEGER EndingTime;
    QueryPerformanceCounter(&EndingTime);

    uint64_t elapsed = EndingTime.QuadPart - basetime.QuadPart;
    elapsed *= 1000000;
    return elapsed / Frequency.QuadPart;
#else
    return std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - basetime)
        .count();
#endif
  }

  static void collect();

 private:
  Tracer() = delete;
  DISALLOW_COPY_AND_ASSIGN(Tracer);

#ifdef _WIN32
  static LARGE_INTEGER basetime;
#else
  static std::chrono::steady_clock::time_point basetime;
#endif
  static std::string filename;
  static bool is_enabled;
  static TraceWriter& instance();
};

class ScopedTracer {
 public:
  explicit ScopedTracer(const std::string& name,
                        const EventCategory category = EventCategory::DEFAULT, json&& args = {}) {
    if (!Tracer::enabled()) {
      is_enabled = false;
      return;
    }

    this->_name = name;
    this->category = category;

    Tracer::begin(name, category, std::move(args));
  }

  ~ScopedTracer() {
    if (!Tracer::enabled() || !is_enabled) return;

    Tracer::end(_name, category);
  }

 private:
  bool is_enabled = true;
  std::string _name;
  EventCategory category;

  DISALLOW_COPY_AND_ASSIGN(ScopedTracer);
};
