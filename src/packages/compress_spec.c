/* Compression efun specifications.
 * Started Wed Mar 21 01:52:25 PST 2001
 * by David Bennett (ddt@discworld.imaginary.com)
 */

int compress_file(string, string|void);
int uncompress_file(string, string|void);

buffer compress(string|buffer);
buffer uncompress(string|buffer);
