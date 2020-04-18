#include "base/package_api.h"

#include "file.h"

#ifdef F_DUMP_TRACE
void f_dump_trace() { push_array(get_svalue_trace()); }
#endif

#ifdef F_TRACE_START
void f_trace_start() {
  auto duration_secs = sp->u.number;
  if (duration_secs < 0 || duration_secs > 5 * 60) {
    error("Invalid duration specified.");
  }

  auto realfile = check_valid_path((sp - 1)->u.string, current_object, "trace_start", 1);

  if (!realfile) {
    error("Permission denied for trace file: %s", (sp - 1)->u.string);
  }

  if (Tracer::enabled()) {
    Tracer::collect();
  }

  // Used later in the block.
  std::string filename(realfile);

  // Register event to start tracing.
  // This is done because we want to skip the rest of current LPC stack.
  add_walltime_event(std::chrono::seconds(0), tick_event::callback_type([=] {
                       Tracer::start(filename.c_str());
                       Tracer::setThreadName("Fluffos Main");

                       // register closure.
                       add_walltime_event(std::chrono::seconds(duration_secs),
                                          tick_event::callback_type([] { Tracer::collect(); }));
                     }));

  pop_2_elems();
}
#endif

#ifdef F_TRACE_END
void f_trace_end() {
  if (!Tracer::enabled()) return;

  Tracer::collect();
}
#endif
