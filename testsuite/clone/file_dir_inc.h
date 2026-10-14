// Included by single/tests/compiler/file_dir_in_include.lpc.
// __FILE__/__DIR__ here must name the including object (historical
// MudOS/FluffOS meaning), not this header under /clone/.
string hdr_file() { return __FILE__; }
string hdr_dir() { return __DIR__; }
