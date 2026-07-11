#ifndef PACKAGES_JSBRIDGE_JSBRIDGE_H
#define PACKAGES_JSBRIDGE_JSBRIDGE_H

/*
 * LPC <-> JavaScript bridge (WASM build only); see jsbridge.cc.
 */

#ifdef DEBUGMALLOC_EXTENSIONS
// Account for refs held by in-flight js_call() callbacks (called from the
// develop package's memory sweep).
void mark_js_calls();
#endif

// Drop all in-flight js_call() state and the refs it holds; called from
// shutdownMudOS() before the tick queues are cleared.
void jsbridge_cleanup();

#endif /* PACKAGES_JSBRIDGE_JSBRIDGE_H */
