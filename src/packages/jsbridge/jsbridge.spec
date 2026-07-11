/*
 * jsbridge.spec: LPC <-> JavaScript bridge (WASM build only).
 */

string js_eval(string);
int js_export(string, void | string | function);
int js_call(string, string *, void | string | function);
