#ifdef __PACKAGE_SHA1__

#define MESSAGE_FILE "/data/sha1/byte-messages.sha1"
#define HASH_FILE    "/data/sha1/byte-hashes.sha1"

string * read_data(string filename);
buffer uncompact(string input);

void do_tests()
{
    string *messages, *hashes;

    /* Validate the hashes in http://www.nsrl.nist.gov/testdata/NSRLvectors.zip */
    messages = read_data(MESSAGE_FILE);
    messages = map(messages, (: uncompact :));

    hashes = read_data(HASH_FILE);
    hashes = map(hashes, (: lower_case :));

    ASSERT(sizeof(messages) > 0);
    ASSERT_EQ(sizeof(messages), sizeof(hashes));

    for (int i = 0; i < sizeof(messages); i++) {
	ASSERT_EQ(sha1(messages[i]), hashes[i]);
    }

    /* Verify that strings work too.  These examples were taken from:
     *     http://en.wikipedia.org/wiki/SHA-1
     *     http://www.nsrl.nist.gov/testdata/
     */
    ASSERT_EQ(sha1(""),
	    "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    ASSERT_EQ(sha1("The quick brown fox jumps over the lazy dog"),
	    "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12");
    ASSERT_EQ(sha1("The quick brown fox jumps over the lazy cog"),
	    "de9f2c7fd25e1b3afad3e85a0bd17d9b100db4b3");
    ASSERT_EQ(sha1("abc"),
	    "a9993e364706816aba3e25717850c26c9cd0d89d");
    ASSERT_EQ(sha1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
	    "84983e441c3bd26ebaae4aa1f95129e5e54670f1");
}

string * read_data(string filename)
{
    string contents, *lines;
   
    contents = read_file(filename);
    contents = replace_string(contents, "\r", "");

    lines = explode(contents, "\n");
    lines = filter(lines, (: $1[<2..] == " ^" :));
    lines = map(lines, (: $1[0..<3] :));

    return lines;
}

/* See /data/sha1/Readme.txt for details on string compaction */
buffer uncompact(string input)
{
    int *counts, b, bits, char, index, count;
    buffer result;

    /* Split up fields; get initial bit value (0 or 1) and array of counts */
    counts = map(explode(input, " "), (: to_int :));
    b = !!counts[1];
    counts = counts[2..];

    /* Count how long the result will be, and allocate a buffer.  Using
     * buffers instead of strings because we need to support '\0'
     */
    foreach (count in counts) {
	bits += count;
    }
    result = allocate_buffer(bits / 8);
    bits = 0;

    foreach (count in counts) {
	while (count > 0) {
	    /* Optimize for all-zero or all-one bytes -- big speedup here */
	    while (count >= 8 && bits == 0) {
		result[index++] = b && 0xff;
		count -= 8;
	    }

	    /* Add bits individually */
	    if (count--) {
		char = (char << 1) | b;
		if (++bits == 8) {
		    result[index++] = char & 0xff;
		    bits = 0;
		}
	    }
	}

	/* Toggle the current bit */
	b = !b;
    }

    return result;
}

#endif
