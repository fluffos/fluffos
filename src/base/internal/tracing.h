#pragma once

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
  Event(std::string_view name, EventCategory category, const char* phase,
        std::optional<json>&& args = std::nullopt);

  unsigned long process_id;
  unsigned long thread_id;
  double timestamp;
  double duration{};
  EventCategory category = EventCategory::DEFAULT;
  const char* phase = "i";
  std::string name = "DEFAULT";
  std::optional<json> args;

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

 public:
  // Move only
  Event(Event&& other) = default;
  Event& operator=(Event&& other) = default;
  // The copy operations are implicitly deleted, but you can
  // spell that out explicitly if you want:
  Event(const Event&) = delete;
  Event& operator=(const Event&) = delete;
};

class Tracer {
 public:
  static void setThreadName(const std::string_view& name);
  static void log(Event&& e);
  static void counter(const std::string_view& name, long n);
  static void counter(const std::string_view& name, std::optional<json>&& args = std::nullopt);
  static void logSimpleEvent(const std::string_view& name, const EventCategory& category);
  static void begin(const std::string_view& name, const EventCategory& category, json&& args);
  static void begin(const std::string_view& name, const EventCategory& category);
  static void end(const std::string_view&, const EventCategory& category);

  static inline void start(const char* file) {
    // If there is already events, flush it out first.
    if (is_enabled) {
      collect();
    }
#ifdef _WIN32
    QueryPerformanceCounter(&basetime);
#else
    basetime = std::chrono::high_resolution_clock::now();
#endif
    filename = file;
    is_enabled = true;
  }

  static inline void stop() { is_enabled = false; }

  static inline bool enabled() { return is_enabled; }

  static inline double timestamp() {
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
    return std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() -
                                                     basetime)
        .count();
#endif
  }

  static void collect();

#ifdef _WIN32
  static LARGE_INTEGER basetime;
#else
  static std::chrono::high_resolution_clock::time_point basetime;
#endif
  static std::string filename;
  static bool is_enabled;
  static TraceWriter& instance();

 public:
  // Not copyable or movable
  Tracer(const Tracer&) = delete;
  Tracer& operator=(const Tracer&) = delete;
  // The move operations are implicitly disabled, but you can
  // spell that out explicitly if you want:
  Tracer(Tracer&&) = delete;
  Tracer& operator=(Tracer&&) = delete;
};

class ScopedTracerInner {
 public:
  ScopedTracerInner(const std::string& name, const EventCategory category = EventCategory::DEFAULT,
                    json&& args = {}, double time_limit_usec = 100);

  virtual ~ScopedTracerInner();

 private:
  double time_limit_usec;
  std::unique_ptr<Event> event;

 public:
  // Not copyable or movable
  ScopedTracerInner(const ScopedTracerInner&) = delete;
  ScopedTracerInner& operator=(const ScopedTracerInner&) = delete;

  // The move operations are implicitly disabled, but you can
  // spell that out explicitly if you want:
  ScopedTracerInner(ScopedTracerInner&&) = delete;
  ScopedTracerInner& operator=(ScopedTracerInner&&) = delete;
};

class ScopedTracer {
 public:
  template <typename... _Arg>
  ScopedTracer(_Arg&&... __arg)
      : _data(Tracer::enabled()
                  ? std::make_optional<ScopedTracerInner>(std::forward<_Arg>(__arg)...)
                  : std::nullopt){};

 private:
  std::optional<ScopedTracerInner> _data;

 public:
  // Not copyable or movable
  ScopedTracer(const ScopedTracer&) = delete;
  ScopedTracer& operator=(const ScopedTracer&) = delete;

  // The move operations are implicitly disabled, but you can
  // spell that out explicitly if you want:
  ScopedTracer(ScopedTracer&&) = delete;
  ScopedTracer& operator=(ScopedTracer&&) = delete;
};
