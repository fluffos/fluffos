#include "base/package_api.h"

#include "file.h"

#ifdef F_DUMP_TRACE
vm::Value f_dump_trace(vm::EfunArgs&) {
  return vm::Value::addref(get_svalue_trace()); /* returns a zero-reference array */
}
#endif

#ifdef F_TRACE_START
void f_trace_start(vm::EfunArgs&, vm::StringArg file, LPC_INT duration) {
  // Unconditional: Tracer::collect() self-guards on an empty filename, and
  // gating it on enabled() is what wedged tracing for the life of the
  // process. When a trace hits MAX_EVENTS, TraceWriter::log() calls
  // Tracer::stop() -- it cannot flush from there, since flush() takes the
  // same lock log() already holds -- so the buffer is left FULL with
  // is_enabled false. Every flush path was then gated behind enabled(), so
  // nothing ever drained it: the next trace_start() re-enabled, logged one
  // event, immediately re-tripped the cap and disabled itself again, and no
  // file was ever written. Silently -- the "dumping N events" line only
  // prints from inside flush(). Recovered only by a restart.
  Tracer::collect();

  auto duration_secs = duration;
  if (duration_secs < 0 || duration_secs > 5 * 60) {
    error("Invalid duration specified.\n");
  }

  const auto* realfile = check_valid_path(file.c_str(), current_object, "trace_start", 1);

  if (!realfile) {
    error("Permission denied for trace file: %s\n", file.c_str());
  }

  // Used later in the block.
  std::string const filename(realfile);

  Tracer::start(filename.c_str());
  Tracer::setThreadName("FluffOS Main");
  // register closure.
  // Also unconditional, and this is the one that matters most: it is the
  // only thing that ends an ordinary trace window, so a trace that overflowed
  // mid-window has to be able to drain here.
  add_walltime_event(std::chrono::seconds(duration_secs),
                     TickEvent::callback_type([] { Tracer::collect(); }));
}
#endif

#ifdef F_TRACE_END
void f_trace_end(vm::EfunArgs&) {
  // No enabled() gate: a trace that filled its buffer is exactly the case
  // that needs ending, and collect() already no-ops when there is nothing
  // pending.
  Tracer::collect();
}
#endif
