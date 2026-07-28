#include <gtest/gtest.h>
#include "base/std.h"

#include "base/internal/tracing.h"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <string>
#include <thread>

// The tracer produces the chrome://tracing JSON the driver writes with
// `driver -t<file>`. Only the "tracing disabled" fast path was covered before,
// so neither the emitted JSON nor the event/category plumbing was tested.

namespace {

// Removes any leftover file from an earlier run: the writer thread is
// asynchronous, so a stale file could otherwise be read as this run's output.
std::string trace_path(const std::string& name) {
  auto path = std::string(testing::TempDir()) + "trace_test_" + name + ".json";
  ::remove(path.c_str());
  return path;
}

// Tracer::collect() hands the buffer to a writer thread, so the file appears
// asynchronously; wait for a complete JSON document.
json read_trace(const std::string& path) {
  for (int i = 0; i < 200; i++) {
    std::ifstream in(path);
    if (in) {
      std::string const content((std::istreambuf_iterator<char>(in)),
                                std::istreambuf_iterator<char>());
      if (!content.empty()) {
        try {
          return json::parse(content);
        } catch (const json::parse_error&) {
          // still being written
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
  }
  ADD_FAILURE() << "trace file never became valid JSON: " << path;
  return json::array();
}

// Names of the events in the trace, in emission order.
std::vector<std::string> names_of(const json& trace) {
  std::vector<std::string> names;
  for (const auto& e : trace) {
    names.push_back(e["name"].get<std::string>());
  }
  return names;
}

const json* find_event(const json& trace, const std::string& name) {
  for (const auto& e : trace) {
    if (e["name"] == name) {
      return &e;
    }
  }
  return nullptr;
}

class TracingTest : public ::testing::Test {
 protected:
  void TearDown() override {
    Tracer::stop();
    Tracer::collect();
  }
};

}  // namespace

TEST_F(TracingTest, DisabledTracerDropsEverything) {
  ASSERT_FALSE(Tracer::enabled());

  Tracer::logSimpleEvent("dropped", EventCategory::DEFAULT);
  Tracer::begin("dropped", EventCategory::DEFAULT);
  Tracer::end("dropped", EventCategory::DEFAULT);
  Tracer::counter("dropped", 1);
  Tracer::setThreadName("dropped");
  { ScopedTracer const _("dropped", EventCategory::DEFAULT); }

  // Nothing was buffered, so collect() has no file to write.
  auto path = trace_path("disabled");
  Tracer::collect();
  std::ifstream in(path);
  EXPECT_FALSE(in.good());
}

TEST_F(TracingTest, StartEnablesAndStopDisables) {
  auto path = trace_path("enable");
  Tracer::start(path.c_str());
  EXPECT_TRUE(Tracer::enabled());
  EXPECT_GE(Tracer::timestamp(), 0.0);

  Tracer::stop();
  EXPECT_FALSE(Tracer::enabled());
  Tracer::logSimpleEvent("after_stop", EventCategory::DEFAULT);

  Tracer::start(path.c_str());
  Tracer::logSimpleEvent("after_restart", EventCategory::DEFAULT);
  Tracer::collect();
  EXPECT_FALSE(Tracer::enabled());

  auto trace = read_trace(path);
  EXPECT_EQ(std::vector<std::string>{"after_restart"}, names_of(trace));
}

TEST_F(TracingTest, WritesEventsWithPhasesAndCategories) {
  auto path = trace_path("events");
  Tracer::start(path.c_str());

  Tracer::setThreadName("unittest");
  Tracer::logSimpleEvent("instant", EventCategory::IO_FS);
  Tracer::begin("scope", EventCategory::VM_COMPILE_FILE);
  Tracer::end("scope", EventCategory::VM_COMPILE_FILE);
  Tracer::begin("scope_with_args", EventCategory::VM_LOAD_OBJECT, json{{"file", "/foo.c"}});
  Tracer::end("scope_with_args", EventCategory::VM_LOAD_OBJECT);
  Tracer::counter("objects", 42);

  Tracer::collect();
  auto trace = read_trace(path);

  ASSERT_EQ(7u, trace.size());

  const auto* thread_name = find_event(trace, "thread_name");
  ASSERT_NE(nullptr, thread_name);
  EXPECT_EQ("M", (*thread_name)["ph"]);
  EXPECT_EQ(0.0, (*thread_name)["ts"]);
  EXPECT_EQ("unittest", (*thread_name)["args"]["name"]);

  const auto* instant = find_event(trace, "instant");
  ASSERT_NE(nullptr, instant);
  EXPECT_EQ("i", (*instant)["ph"]);
  EXPECT_EQ("IO: Filesystem", (*instant)["cat"]);
  EXPECT_GT((*instant)["ts"].get<double>(), 0.0);
  EXPECT_EQ(trace[0]["pid"], (*instant)["pid"]);
  EXPECT_EQ(trace[0]["tid"], (*instant)["tid"]);

  EXPECT_EQ("B", trace[2]["ph"]);
  EXPECT_EQ("VM: Compile File", trace[2]["cat"]);
  EXPECT_EQ("E", trace[3]["ph"]);

  const auto* with_args = find_event(trace, "scope_with_args");
  ASSERT_NE(nullptr, with_args);
  EXPECT_EQ("/foo.c", (*with_args)["args"]["file"]);

  // a counter event's args must be a {name: value} object for the trace viewer
  // to plot it
  const auto* counter = find_event(trace, "objects");
  ASSERT_NE(nullptr, counter);
  EXPECT_EQ("C", (*counter)["ph"]);
  ASSERT_TRUE((*counter)["args"].is_object());
  EXPECT_EQ(42, (*counter)["args"]["objects"]);
}

// ScopedTracer logs a single "X" (complete) event, and only when the scope
// lasted at least time_limit_usec.
TEST_F(TracingTest, ScopedTracerLogsCompleteEventsOverTheTimeLimit) {
  auto path = trace_path("scoped");
  Tracer::start(path.c_str());

  {
    ScopedTracer const _("fast_but_reported", EventCategory::LPC_FUNCTION, std::nullopt,
                         /* time_limit_usec = */ 0);
  }
  {
    ScopedTracer const _("too_fast_to_report", EventCategory::LPC_EFUN, std::nullopt,
                         /* time_limit_usec = */ 1e9);
  }
  {
    ScopedTracer const _("with_lazy_args", EventCategory::LPC_CATCH,
                         std::make_optional<std::function<json()>>([]() {
                           return json{{"lazy", true}};
                         }),
                         0);
  }

  Tracer::collect();
  auto trace = read_trace(path);

  ASSERT_EQ(2u, trace.size());
  EXPECT_EQ("fast_but_reported", trace[0]["name"]);
  EXPECT_EQ("X", trace[0]["ph"]);
  EXPECT_EQ("LPC Function", trace[0]["cat"]);
  EXPECT_GE(trace[0]["dur"].get<double>(), 0.0);

  EXPECT_EQ("with_lazy_args", trace[1]["name"]);
  EXPECT_EQ(true, trace[1]["args"]["lazy"]);
}

// start() on an already-tracing session flushes the pending events first.
TEST_F(TracingTest, RestartFlushesPreviousTrace) {
  auto first = trace_path("restart_first");
  auto second = trace_path("restart_second");

  Tracer::start(first.c_str());
  Tracer::logSimpleEvent("first_session", EventCategory::DEFAULT);
  Tracer::start(second.c_str());
  Tracer::logSimpleEvent("second_session", EventCategory::DEFAULT);
  Tracer::collect();

  EXPECT_EQ(std::vector<std::string>{"first_session"}, names_of(read_trace(first)));
  EXPECT_EQ(std::vector<std::string>{"second_session"}, names_of(read_trace(second)));
}

TEST(TracingEvent, CategoryNamesCoverEveryCategory) {
  auto name_of = [](EventCategory category) {
    Event e("event", category, "i");
    return std::string(e.category_name());
  };

  EXPECT_EQ("__metadata", name_of(EventCategory::METADATA));
  EXPECT_EQ("DEFAULT", name_of(EventCategory::DEFAULT));
  EXPECT_EQ("IO: Filesystem", name_of(EventCategory::IO_FS));
  EXPECT_EQ("VM: Compile File", name_of(EventCategory::VM_COMPILE_FILE));
  EXPECT_EQ("VM: Load Object", name_of(EventCategory::VM_LOAD_OBJECT));
  EXPECT_EQ("Apply cache lookup", name_of(EventCategory::APPLY_CACHE));
  EXPECT_EQ("LPC Catch", name_of(EventCategory::LPC_CATCH));
  EXPECT_EQ("LPC Function", name_of(EventCategory::LPC_FUNCTION));
  EXPECT_EQ("LPC EFUN", name_of(EventCategory::LPC_EFUN));
  // The "Unknown" fallback branch (an EventCategory value outside the
  // declared enumerators) is intentionally not exercised here: constructing
  // one via static_cast<EventCategory>(999) is a load of an invalid enum
  // value, which is undefined behavior in its own right (flagged by
  // -fsanitize=enum) independent of whatever category_name() does with it.
  // Low value relative to the UBSan cost -- category_name()'s default case
  // is a one-line literal, not logic worth a dedicated regression test.
}

TEST(TracingEvent, EventCarriesProcessAndThreadIdentity) {
  Event e("named", EventCategory::DEFAULT, "B", json{{"k", "v"}});

  EXPECT_EQ("named", e.name);
  EXPECT_STREQ("B", e.phase);
  EXPECT_EQ(EventCategory::DEFAULT, e.category);
  EXPECT_GT(e.process_id, 0u);
  ASSERT_TRUE(e.args.has_value());
  EXPECT_EQ("v", (*e.args)["k"]);
}
