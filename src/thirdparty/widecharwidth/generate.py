#!/usr/bin/env python

""" Outputs the width file to stdout. """

import datetime
import hashlib
import os.path
import re
import sys
try:
  #python3
  from urllib.request import urlretrieve
except ImportError:
  from urllib import urlretrieve
try:
  #python3
  xrange
except NameError:
  xrange = range

VERSION = "12.1.0"
UNICODE_DATA_URL = 'https://unicode.org/Public/%s/ucd/UnicodeData.txt' % VERSION
EAW_URL = 'https://unicode.org/Public/%s/ucd/EastAsianWidth.txt' % VERSION
EMOJI_DATA_URL = 'https://unicode.org/Public/emoji/12.1/emoji-data.txt'

# A handful of field names
# See https://www.unicode.org/L2/L1999/UnicodeData.html
FIELD_CODEPOINT = 0
FIELD_NAME = 1
FIELD_CATEGORY = 2

# Category for unassigned codepoints.
CAT_UNASSIGNED = 'Cn'

# Category for private use codepoints.
CAT_PRIVATE_USE = 'Co'

# Category for surrogates.
CAT_SURROGATE = 'Cs'

# Maximum codepoint value.
MAX_CODEPOINT = 0x110000

CPP_PREFIX = 'widechar_'

OUTPUT_FILENAME = 'widechar_width.h'

OUTPUT_TEMPLATE = r'''
/**
 * {filename}, generated on {today}.
 * See https://github.com/ridiculousfish/widecharwidth/
 *
 * SHA1 file hashes:
 *  UnicodeData.txt:     {unicode_hash}
 *  EastAsianWidth.txt:  {eaw_hash}
 *  emoji-data.txt:      {emoji_hash}
 */

#ifndef WIDECHAR_WIDTH_H
#define WIDECHAR_WIDTH_H

#include <algorithm>
#include <iterator>
#include <cstddef>
#include <cstdint>

namespace {{

/* Special width values */
enum {{
  {p}nonprint = -1,     // The character is not printable.
  {p}combining = -2,    // The character is a zero-width combiner.
  {p}ambiguous = -3,    // The character is East-Asian ambiguous width.
  {p}private_use = -4,  // The character is for private use.
  {p}unassigned = -5,   // The character is unassigned.
  {p}widened_in_9 = -6  // Width is 1 in Unicode 8, 2 in Unicode 9+.
}};

/* An inclusive range of characters. */
struct {p}range {{
  int32_t lo;
  int32_t hi;
}};

/* Simple ASCII characters - used a lot, so we check them first. */
static const struct {p}range {p}ascii_table[] = {{
    {ascii}
}};

/* Private usage range. */
static const struct {p}range {p}private_table[] = {{
    {private}
}};

/* Nonprinting characters. */
static const struct {p}range {p}nonprint_table[] = {{
    {nonprint}
}};

/* Width 0 combining marks. */
static const struct {p}range {p}combining_table[] = {{
    {combining}
}};

/* Width.2 characters. */
static const struct {p}range {p}doublewide_table[] = {{
    {doublewide}
}};

/* Ambiguous-width characters. */
static const struct {p}range {p}ambiguous_table[] = {{
    {ambiguous}
}};


/* Unassigned characters. */
static const struct {p}range {p}unassigned_table[] = {{
    {unassigned}
}};

/* Characters that were widened from with 1 to 2 in Unicode 9. */
static const struct {p}range {p}widened_table[] = {{
    {widenedin9}
}};

template<typename Collection>
bool {p}in_table(const Collection &arr, int32_t c) {{
    auto where = std::lower_bound(std::begin(arr), std::end(arr), c,
        []({p}range p, int32_t c) {{ return p.hi < c; }});
    return where != std::end(arr) && where->lo <= c;
}}

/* Return the width of character c, or a special negative value. */
int {p}wcwidth(int32_t c) {{
    if ({p}in_table({p}ascii_table, c))
        return 1;
    if ({p}in_table({p}private_table, c))
        return {p}private_use;
    if ({p}in_table({p}nonprint_table, c))
        return {p}nonprint;
    if ({p}in_table({p}combining_table, c))
        return {p}combining;
    if ({p}in_table({p}doublewide_table, c))
        return 2;
    if ({p}in_table({p}ambiguous_table, c))
        return {p}ambiguous;
    if ({p}in_table({p}unassigned_table, c))
        return {p}unassigned;
    if ({p}in_table({p}widened_table, c))
        return {p}widened_in_9;
    return 1;
}}

}} // namespace
#endif // WIDECHAR_WIDTH_H
'''

# Ambiguous East Asian characters
WIDTH_AMBIGUOUS_EASTASIAN = -3

# Width changed from 1 to 2 in Unicode 9.0
WIDTH_WIDENED_IN_9 = -6

class CodePoint(object): # pylint: disable=too-few-public-methods
  """ Represents a single Unicode codepoint """
  def __init__(self, codepoint):
    self.codepoint = codepoint
    self.width = None
    self.category = CAT_UNASSIGNED

  def hex(self):
    """ Return the codepoint as a hex string """
    return '0x%05X' % self.codepoint


def log(msg):
  """ Logs a string to stderr """
  sys.stderr.write(str(msg) + '\n')


def read_datafile(url):
  """ Download a file from url to name if not already present.
      Return the file as a tuple (lines, sha1)
      lines will have comment-only lines removed, sha1 is a string.
  """
  name = url.rsplit('/', 1)[-1]
  if not os.path.isfile(name):
    log("Downloading " + name)
    urlretrieve(url, name)
  with open(name, 'rb') as ofile:
    data = ofile.read()
  hashval = hashlib.sha1(data).hexdigest()
  lines = data.decode("utf-8").split('\n')
  lines = [line for line in lines if not line.startswith('#')]
  return (lines, hashval)


def set_general_categories(unicode_data, cps):
  """ Receives lines from UnicodeData.txt,
      and sets general categories for codepoints. """
  for line in unicode_data:
    fields = line.strip().split(';')
    if len(fields) > FIELD_CATEGORY:
      for idx in hexrange_to_range(fields[FIELD_CODEPOINT]):
        cps[idx].category = fields[FIELD_CATEGORY]


def merged_codepoints(cps):
  """ return a list of codepoints (start, end) for inclusive ranges """
  if not cps:
    return []
  cps = sorted(cps, key=lambda cp: cp.codepoint)
  ranges = [(cps[0], cps[0])]
  for cp in cps[1:]:
    last_range = ranges[-1]
    if cp.codepoint == last_range[1].codepoint + 1:
      ranges[-1] = (last_range[0], cp)
      continue
    ranges.append((cp, cp))
  return ranges


def gen_seps(length):
  """ Yield separators for a table of given length """
  table_columns = 3
  for idx in xrange(1, length + 1):
    if idx == length:
      yield ''
    elif idx % table_columns == 0:
      yield ',\n    '
    else:
      yield ', '


def codepoints_to_carray_str(cps):
  """ Given a list of codepoints, return a C array string representing their inclusive ranges. """
  result = ''
  ranges = merged_codepoints(cps)
  seps = gen_seps(len(ranges))
  for (start, end) in ranges:
    result += '{%s, %s}%s' % (start.hex(), end.hex(), next(seps))
  return result


def hexrange_to_range(hexrange):
  """ Given a string like 1F300..1F320 representing an inclusive range,
      return the range of codepoints.
      If the string is like 1F321, return a range of just that element.
  """
  fields = [int(val, 16) for val in hexrange.split('..')]
  if len(fields) == 1:
    fields += fields
  return range(fields[0], fields[1] + 1)


def parse_eaw_line(eaw_line):
  """ Return a list of tuples (codepoint, width) from an EAW line """
  # Remove hash
  eaw_line = eaw_line.split('#', 1)[0]
  fields = eaw_line.strip().split(';')
  if len(fields) != 2:
    return []
  cps, width_type = fields
  # width_types:
  #  A: ambiguous, F: fullwidth, H: halfwidth,
  #. N: neutral, Na: east-asian Narrow
  if width_type == 'A':
    width = WIDTH_AMBIGUOUS_EASTASIAN
  elif width_type in ['F', 'W']:
    width = 2
  else:
    width = 1
  return [(cp, width) for cp in hexrange_to_range(cps)]


def set_eaw_widths(eaw_data_lines, cps):
  """ Read from EastAsianWidth.txt, set width values on the codepoints """
  for line in eaw_data_lines:
    for (cp, width) in parse_eaw_line(line):
      cps[cp].width = width
  # Apply the following special cases:
  #  - The unassigned code points in the following blocks default to "W":
  #         CJK Unified Ideographs Extension A: U+3400..U+4DBF
  #         CJK Unified Ideographs:             U+4E00..U+9FFF
  #         CJK Compatibility Ideographs:       U+F900..U+FAFF
  #  - All undesignated code points in Planes 2 and 3, whether inside or
  #      outside of allocated blocks, default to "W":
  #         Plane 2:                            U+20000..U+2FFFD
  #         Plane 3:                            U+30000..U+3FFFD
  wide_ranges = [(0x3400, 0x4DBF), (0x4E00, 0x9FFF), (0xF900, 0xFAFF),
                 (0x20000, 0x2FFFD), (0x30000, 0x3FFFD)]
  for wr in wide_ranges:
    for cp in xrange(wr[0], wr[1] + 1):
      if cps[cp].width is None:
        cps[cp].width = 2


def parse_emoji_line(line):
  """ Return a list {cp, version} for the line """
  # Example line: 0023   ; Emoji #  1.1  [1] (#)  number sign
  fields_comment = line.split('#', 1)
  if len(fields_comment) != 2:
    return []
  fields, comment = fields_comment
  cps, _prop = fields.split(';')
  version = 0.0
  fmtre = re.search(r'^\s*\d+\.\d+', comment)
  # In later versions of emoji-data.txt there are some "reserved"
  # entries that have "NA" instead of a Unicode version number
  # of first use, they will now return a zero version instead of
  # crashing the script
  if fmtre is None:
    return [(cp, version) for cp in hexrange_to_range(cps)]

  version = float(re.search(r'^\s*\d+\.\d+', comment).group(0))
  return [(cp, version) for cp in hexrange_to_range(cps)]


def set_emoji_widths(emoji_data_lines, cps):
  """ Read from emoji-data.txt, set codepoint widths """
  for line in emoji_data_lines:
    for (cp, version) in parse_emoji_line(line):
      # Don't consider <=1F000 values as emoji. These can only be made
      # emoji through the variation selector which interacts terribly
      # with wcwidth().
      # Skip codepoints that have a version of 0.0 as they were marked
      # in the emoji-data file as reserved/unused:
      if cp >= 0x1F000:
        if version > 1.0:
          cps[cp].width = 2 if version >= 9.0 else WIDTH_WIDENED_IN_9

def set_hardcoded_ranges(cps):
  """ Mark private use and surrogate codepoints """
  # Private use can be determined awkwardly from UnicodeData.txt,
  # but we just hard-code them.
  # We do not treat "private use high surrogate" as private use
  # so as to match wcwidth9().
  private_ranges = [(0xE000, 0xF8FF), (0xF0000, 0xFFFFD), (0x100000, 0x10FFFD)]
  for (first, last) in private_ranges:
    for idx in xrange(first, last+1):
      cps[idx].category = CAT_PRIVATE_USE

  surrogate_ranges = [(0xD800, 0xDBFF), (0xDC00, 0xDFFF)]
  for (first, last) in surrogate_ranges:
    for idx in xrange(first, last+1):
      cps[idx].category = CAT_SURROGATE


def generate():
  """ Return our widechar_width.h as a string """
  # Read our three files.
  unicode_data, unicode_hash = read_datafile(UNICODE_DATA_URL)
  eaw_data, eaw_hash = read_datafile(EAW_URL)
  emoji_data, emoji_hash = read_datafile(EMOJI_DATA_URL)

  log("Thinking...")

  # Generate a CodePoint for each value.
  cps = [CodePoint(i) for i in xrange(MAX_CODEPOINT+1)]

  set_general_categories(unicode_data, cps)
  set_eaw_widths(eaw_data, cps)
  set_emoji_widths(emoji_data, cps)
  set_hardcoded_ranges(cps)

  def categories(cats):
    """ Return a carray string of codepoints contained in any of the given
        categories. """
    catset = set(cats)
    matches = [cp for cp in cps if cp.category in catset]
    return codepoints_to_carray_str(matches)

  def codepoints_with_width(width):
    """ Return a carray string of codepoints with the given width. """
    return codepoints_to_carray_str([cp for cp in cps if cp.width == width])

  def ascii_codepoints():
    """ Return a carray string of codepoints with the given width. """
    return codepoints_to_carray_str([cp for cp in cps if cp.codepoint < 0x7F and cp.codepoint >= 0x20])

  fields = {
      'p': CPP_PREFIX,
      'filename': OUTPUT_FILENAME,
      'today': str(datetime.date.today()),
      'unicode_hash': unicode_hash,
      'eaw_hash': eaw_hash,
      'emoji_hash': emoji_hash,
      'ascii': ascii_codepoints(),
      'private': categories([CAT_PRIVATE_USE]),
      'nonprint': categories(['Cc', 'Cf', 'Zl', 'Zp', CAT_SURROGATE]),
      'combining': categories(['Mn', 'Mc', 'Me']),
      'doublewide': codepoints_with_width(2),
      'unassigned': categories([CAT_UNASSIGNED]),
      'ambiguous': codepoints_with_width(WIDTH_AMBIGUOUS_EASTASIAN),
      'widenedin9': codepoints_with_width(WIDTH_WIDENED_IN_9),
  }
  return OUTPUT_TEMPLATE.strip().format(**fields)

if __name__ == '__main__':
  with open(OUTPUT_FILENAME, 'w') as fd:
    fd.write(generate())
    fd.write('\n')
  log("Output " + OUTPUT_FILENAME)
