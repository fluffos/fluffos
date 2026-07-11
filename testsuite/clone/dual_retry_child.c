// Fixture for /single/tests/efuns/dual_extension.lpc: the ".c" spelling
// inherits an UNLOADED parent, forcing load_object's inherit-retry loop
// to reload this object -- the retry must keep the caller's exact ".c"
// spelling (the .lpc twin has no inherit and answers "lpc").
inherit "/clone/dual_retry_parent";
string which() { return "c"; }
