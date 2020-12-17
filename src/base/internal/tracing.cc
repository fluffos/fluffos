// Yucong Sun (sunyucong@gmail.com)
//
// Generate driver tracing data to be viewed in chrome http://about:tracing

#include <vector>
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

Event::Event(std::string_view name, EventCategory category, const char* phase,
             std::optional<json>&& args)
    : process_id(::_get_current_process_id()),
      thread_id(::_get_current_thread_id()),
      timestamp(Tracer::timestamp()),
      category(category),
      phase(phase),
      name(name),
      args(args) {}

class TraceWriter {
 public:
  ~TraceWriter();

  void log(Event&& e) {
    std::lock_guard<std::mutex> _guard(lock);

    if (!buffer) {
      buffer = std::make_unique<std::vector<Event>>();
      buffer->reserve(MAX_EVENTS);
    }

    if (buffer->size() >= MAX_EVENTS) {
      Tracer::stop();
    }

    buffer->push_back(std::move(e));
  }
  void flush(const std::string& file);

 private:
  std::mutex lock;
  std::unique_ptr<std::vector<Event>> buffer;
  std::vector<std::thread> dump_threads;
};

TraceWriter::~TraceWriter() {
  std::lock_guard<std::mutex> _lock(lock);
  if (buffer && !buffer->empty()) {
    debug_message("Uncollected profiling events: %ld.\n", buffer->size());
  }
  for (auto& t : dump_threads) {
    if (t.joinable()) {
      t.join();
    }
  }
}

void TraceWriter::flush(const std::string& filename) {
  std::lock_guard<std::mutex> _guard(lock);

  if (!buffer || buffer->empty()) {
    return;
  }

  debug_message("Trace duration: %lf us, dumping %ld events to %s in separate thread.\n",
                Tracer::timestamp(), buffer->size(), filename.c_str());

  this->dump_threads.emplace_back([current_buffer = move(buffer), filename] {
    auto begin = std::chrono::high_resolution_clock::now();

    std::ofstream file(filename, std::ofstream::out | std::ofstream::binary);

    if (!file) {
      debug_message("Error opening file %s: .\n", filename.c_str());
      current_buffer->clear();
      return;
    }

    file << "[";

    bool is_first = true;

    for (auto& e : *current_buffer) {
      if (is_first)
        is_first = false;
      else
        file << ",";
      file << "\n";  // use std::endl flush the buffer, best to avoid.

      file << "{"
           << R"("pid":)" << e.process_id << ","
           << R"("tid":)" << e.thread_id << ","
           << R"("ts":)" << e.timestamp << ","
           << R"("dur":)" << e.duration << ","
           << R"("ph":")" << e.phase << "\""
           << ","
           << R"("cat":")" << e.category_name() << "\""
           << ","
           << R"("name":)" << json(e.name);

      if (e.phase[0] == 'X') {
        file << ","
             << R"("dur":)" << e.duration;
      }

      if (e.args && !e.args->empty()) {
        file << ","
             << R"("args":)" << *e.args;
      }
      file << "}";
    }

    file << "\n"
         << "]";

    auto dur_us = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::high_resolution_clock::now() - begin)
                      .count();

    debug_message("[thread %lud]: Dump trace successfully to file %s, cost %lld ms.\n",
                  _get_current_thread_id(), filename.c_str(), dur_us);
  });
}

bool Tracer::is_enabled = false;
std::string Tracer::filename;

#ifdef _WIN32
LARGE_INTEGER Tracer::basetime;
#else
std::chrono::high_resolution_clock::time_point Tracer::basetime;
#endif

void Tracer::log(Event&& e) {
  if (Tracer::enabled()) {
    instance().log(std::move(e));
  }
}

void Tracer::logSimpleEvent(const std::string_view& name, const EventCategory& category) {
  if (Tracer::enabled()) {
    log({name, category, "i"});
  }
}
void Tracer::begin(const std::string_view& name, const EventCategory& category, json&& args) {
  if (Tracer::enabled()) {
    log({name, category, "B", std::move(args)});
  }
}
void Tracer::begin(const std::string_view& name, const EventCategory& category) {
  if (Tracer::enabled()) {
    log({name, category, "B"});
  }
}
void Tracer::end(const std::string_view& name, const EventCategory& category) {
  if (Tracer::enabled()) {
    log({name, category, "E"});
  }
}

void Tracer::setThreadName(const std::string_view& name) {
  if (Tracer::enabled()) {
    Event e("thread_name", EventCategory::DEFAULT, "M",
            json{
                {"name", name},
            });
    e.timestamp = 0;
    log(std::move(e));
  }
}

void Tracer::counter(const std::string_view& name, long n) {
  if (Tracer::enabled()) {
    counter(name, {{name, n}});
  }
}

void Tracer::counter(const std::string_view& name, std::optional<json>&& args) {
  if (Tracer::enabled()) {
    log({name, EventCategory::DEFAULT, "C", std::move(args)});
  }
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

ScopedTracerInner::ScopedTracerInner(const std::string& name, const EventCategory category,
                                     json&& args, double time_limit_usec)
    : time_limit_usec(time_limit_usec),
      event(std::make_unique<Event>(name, category, "X", std::move(args))) {}

ScopedTracerInner::~ScopedTracerInner() {
  if (!this->event) return;

  this->event->duration = Tracer::timestamp() - this->event->timestamp;
  if (this->event->duration >= time_limit_usec) {
    Tracer::log(std::move(*this->event));
  }
}
