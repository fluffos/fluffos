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
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace {
const int MAX_EVENTS = 1'000'000;

unsigned long get_current_process_id() {
  static unsigned long const current_process_id =
#ifdef _WIN32
      GetCurrentProcessId();
#else
      ::getpid();
#endif
  return current_process_id;
}

unsigned long thread_id_to_string(std::thread::id id) {
  std::ostringstream os;
  os << id;
  std::string const res = os.str();

  return std::strtoul(res.c_str(), nullptr, 10);
}

unsigned long get_current_thread_id() {
  static thread_local unsigned long const current_thread_id =
      thread_id_to_string(std::this_thread::get_id());
  return current_thread_id;
}

}  // namespace

Event::Event(std::string_view name, EventCategory category, const char* phase,
             std::optional<json>&& args)
    : process_id(::get_current_process_id()),
      thread_id(::get_current_thread_id()),
      timestamp(Tracer::timestamp()),
      category(category),
      phase(phase),
      name(name),
      args(args) {}

class TraceWriter {
 public:
  ~TraceWriter();

  void log(Event&& e) {
    std::lock_guard<std::mutex> const guard(lock_);

    if (!buffer_) {
      buffer_ = std::make_unique<std::vector<Event>>();
      buffer_->reserve(MAX_EVENTS);
    }

    if (buffer_->size() >= MAX_EVENTS) {
      Tracer::stop();
    }

    buffer_->push_back(std::move(e));
  }
  void flush(const std::string& file);

 private:
  // debug_message() is main-thread only, so a dump thread never logs: it
  // queues what happened here and the main thread reports it.
  struct DumpDone {
    std::string filename;
    long long ms;
  };
  void report_finished_dumps();

  std::mutex lock_;
  std::unique_ptr<std::vector<Event>> buffer_;
  std::vector<std::thread> dump_threads_;
  std::mutex done_lock_;
  std::vector<DumpDone> done_;
};

void TraceWriter::report_finished_dumps() {
  std::vector<DumpDone> done;
  {
    std::lock_guard<std::mutex> const guard(done_lock_);
    done.swap(done_);
  }
  for (const auto& d : done) {
    debug_message("Dumped trace to file %s, cost %lld ms.\n", d.filename.c_str(), d.ms);
  }
}

TraceWriter::~TraceWriter() {
  std::lock_guard<std::mutex> const lock(lock_);
  if (buffer_ && !buffer_->empty()) {
    debug_message("Uncollected profiling events: %ld.\n", buffer_->size());
  }
  for (auto& t : dump_threads_) {
    if (t.joinable()) {
      t.join();
    }
  }
  dump_threads_.clear();
  report_finished_dumps();
}

void TraceWriter::flush(const std::string& filename) {
  report_finished_dumps();

  std::lock_guard<std::mutex> const guard(lock_);

  if (!buffer_ || buffer_->empty()) {
    return;
  }

  // Open on the calling thread so a failure is logged from here.
  auto file_ptr =
      std::make_shared<std::ofstream>(filename, std::ofstream::out | std::ofstream::binary);
  if (!*file_ptr) {
    debug_message("Error opening file %s: .\n", filename.c_str());
    buffer_.reset();
    return;
  }

  debug_message("Trace duration: %lf us, dumping %ld events to %s in separate thread.\n",
                Tracer::timestamp(), buffer_->size(), filename.c_str());

  auto dump = [this, current_buffer = std::move(buffer_), file_ptr = std::move(file_ptr),
               filename] {
    auto begin = std::chrono::high_resolution_clock::now();

    std::ofstream& file = *file_ptr;

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

    file.close();

    auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::high_resolution_clock::now() - begin)
                      .count();

    std::lock_guard<std::mutex> const guard(done_lock_);
    done_.push_back({filename, static_cast<long long>(dur_ms)});
  };

#ifdef __EMSCRIPTEN__
  // No threads on WASM: write the trace synchronously.
  dump();
  report_finished_dumps();
#else
  this->dump_threads_.emplace_back(std::move(dump));
#endif
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
    // an explicit std::string key is required, otherwise the initializer list
    // becomes a JSON array instead of the {name: value} object counter tracks
    // are read from.
    counter(name, std::optional<json>(json{{std::string(name), n}}));
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
  static TraceWriter trace_writer;
  return trace_writer;
}

ScopedTracerInner::ScopedTracerInner(const std::string& name, const EventCategory category,
                                     std::optional<std::function<json()>> lazy_arg,
                                     double time_limit_usec)
    : time_limit_usec(time_limit_usec),
      event(std::make_unique<Event>(
          name, category, "X", lazy_arg ? std::make_optional<json>((*lazy_arg)()) : std::nullopt)) {
}

ScopedTracerInner::~ScopedTracerInner() {
  if (!this->event) return;

  this->event->duration = Tracer::timestamp() - this->event->timestamp;
  if (this->event->duration >= time_limit_usec) {
    Tracer::log(std::move(*this->event));
  }
}
