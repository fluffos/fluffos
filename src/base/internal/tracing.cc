// Yucong Sun (sunyucong@gmail.com)
//
// Generate driver tracing data to be viewed in chrome http://about:tracing

#include <deque>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <sys/types.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "tracing.h"

#include "base/internal/log.h"
#include "thirdparty/json/single_include/nlohmann/json.hpp"
using json = nlohmann::json;

namespace {
const int MAX_EVENTS = 1'000'000;

unsigned long _get_current_process_id() {
  static unsigned long _current_process_id =
#ifdef _WIN32
      GetCurrentProcessId();
#else
      ::getpid();
#endif
  return _current_process_id;
}

unsigned long thread_id_to_string(std::thread::id id) {
  std::ostringstream os;
  os << id;
  std::string res = os.str();

  return std::strtoul(res.c_str(), nullptr, 10);
}

unsigned long _get_current_thread_id() {
  static thread_local unsigned long _current_thread_id =
      thread_id_to_string(std::this_thread::get_id());
  return _current_thread_id;
}

}  // namespace

Event::Event()
    : process_id(::_get_current_process_id()),
      thread_id(::_get_current_thread_id()),
      timestamp(Tracer::timestamp()) {}

class TraceWriter {
 public:
  ~TraceWriter();

  void log(const Event& e) {
    std::lock_guard<std::mutex> _guard(lock);

    if (buffer.size() >= MAX_EVENTS) {
      Tracer::stop();
    }

    buffer.push_back(e);
  }
  void flush(const std::string& file);

 private:
  std::mutex lock;
  std::deque<Event> buffer;
};

TraceWriter::~TraceWriter() {
  std::lock_guard<std::mutex> _lock(lock);
  if (!buffer.empty()) {
    debug_message("Uncollected profiling events: %ld.\n", buffer.size());
  }
}

void TraceWriter::flush(const std::string& filename) {
  std::ofstream file;
  file.open(filename);

  file << "[";
  {
    std::lock_guard<std::mutex> _guard(lock);

    debug_message("Dumping %ld events to %s.\n", buffer.size(), filename.c_str());

    bool is_first = true;

    for (auto& e : buffer) {
      if (is_first)
        is_first = false;
      else
        file << ",";
      file << std::endl;

      json item = {
          {"cat", e.category_name()}, {"pid", e.process_id}, {"tid", e.thread_id},
          {"ts", e.timestamp},        {"ph", e.phase},       {"name", e.name},
      };
      if (!e.args.empty()) {
        item["args"] = std::move(e.args);
      }
      file << item;
    }

    buffer.clear();
    buffer.shrink_to_fit();
  }

  file << std::endl << "]";

  file.close();
}

bool Tracer::is_enabled = false;
std::string Tracer::filename;

#ifdef _WIN32
LARGE_INTEGER Tracer::basetime;
#else
std::chrono::steady_clock::time_point Tracer::basetime;
#endif

void Tracer::log(const Event& e) {
  if (Tracer::enabled()) {
    instance().log(e);
  }
}

void Tracer::logSimpleEvent(const std::string& name, const EventCategory& category) {
  Event e;
  e.name = name;
  e.category = category;
  e.phase = "i";

  log(e);
}
void Tracer::begin(const std::string& name, const EventCategory& category, json&& args) {
  Event e;
  e.name = name;
  e.category = category;
  e.phase = "B";
  e.args = std::move(args);
  log(e);
}
void Tracer::end(const std::string& name, const EventCategory& category) {
  Event e;
  e.name = name;
  e.category = category;
  e.phase = "E";
  log(e);
}

void Tracer::setThreadName(const std::string& name) {
  Event e;

  e.name = "thread_name";
  e.category = EventCategory::DEFAULT;
  e.phase = "M";
  e.timestamp = 0;
  e.args = {
      {"name", name},
  };
  log(e);
}

void Tracer::counter(const std::string& name, long n) { counter(name, {{name, n}}); }

void Tracer::counter(const std::string& name, const json& args) {
  Event e;
  e.name = name;
  e.category = EventCategory::DEFAULT;
  e.phase = "C";
  e.args = args;
  log(e);
}

void Tracer::collect() {
  // It's possible that we are over limit and collection was disabled.
  if (!filename.empty()) {
    instance().flush(filename);

    filename.clear();
    is_enabled = false;
  }
}

TraceWriter& Tracer::instance() {
  static TraceWriter _trace_writer;
  return _trace_writer;
}
