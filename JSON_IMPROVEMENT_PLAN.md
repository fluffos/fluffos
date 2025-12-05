# JSON Implementation Improvement Plan

## Current Performance Baseline
- Small object: 13,077 ns/op
- Simple array: 19,046 ns/op
- Nested objects: 15,579 ns/op
- Unicode strings: 16,038 ns/op

## Phase 1: Code Cleanup & Deduplication
**Priority: High | Impact: Medium**

1. **Remove duplicate whitespace functions**
   - Currently have both `json_decode_skip_whitespaces()` (old) and `json_decode_skip_ws()` (new)
   - Delete the old one, ensure all callers use the new optimized version
   - Lines: 77-87 (old function)

2. **Remove unused variables**
   - `json_decode_parse_number()`: variable `from` is declared but unused (line 459)
   - `json_decode_parse_object()`: variable `out` is unused in some paths (line 173)

3. **Remove dead helper functions**
   - `json_decode_parse_next_char()`, `json_decode_parse_next_chars()`, `json_decode_parse_next_line()`
   - These were from the old implementation and are no longer used

## Phase 2: Decoding Optimizations
**Priority: High | Impact: High**

4. **Optimize hex digit conversion**
   - Current implementation uses a 36-case switch statement (lines 90-124)
   - Replace with arithmetic: `(ch >= '0' && ch <= '9') ? (ch - '0') : ...`
   - Estimated improvement: 20-30% for Unicode-heavy JSON

5. **Optimize string parsing**
   - Pre-allocate buffer based on remaining input size instead of fixed 256 bytes
   - Reduce buffer reallocation overhead
   - Estimated improvement: 10-15% for large strings

6. **Optimize number parsing**
   - Remove redundant position tracking (`from` variable)
   - Use direct character-to-digit conversion instead of buffer accumulation
   - Consider using `sscanf()` for validation if available
   - Estimated improvement: 15-20%

7. **Cache parse state locally**
   - Extract `parse[JSON_DECODE_PARSE_POS]` to local variable in hot loops
   - Reduces array indexing overhead
   - Estimated improvement: 5-10% overall

8. **Optimize token parsing (true/false/null)**
   - Current implementation manually checks each character
   - Use `parse_at_token()` helper OR inline compare all bytes at once
   - Estimated improvement: 30-40% for boolean/null heavy JSON

## Phase 3: Encoding Optimizations
**Priority: Medium | Impact: High**

9. **Optimize string encoding**
   - Pre-calculate required buffer size in first pass
   - Avoid character-by-character sprintf for hex digits
   - Use lookup table for hex conversion: `HEX_CHARS[ch >> 4]` and `HEX_CHARS[ch & 0xF]`
   - Estimated improvement: 25-35%

10. **Optimize collection encoding**
    - Pre-allocate result array for mappings/arrays
    - Use buffer-based encoding instead of string concatenation
    - Estimated improvement: 20-30%

11. **Add special case for ASCII-only strings**
    - Detect if string contains only ASCII (0x20-0x7E)
    - Skip expensive character-by-character encoding
    - Estimated improvement: 40-50% for ASCII-only strings

## Phase 4: Advanced Optimizations
**Priority: Low | Impact: Medium**

12. **Add string interning for object keys**
    - Common JSON keys repeat frequently
    - Cache decoded key strings
    - Estimated improvement: 10-15% for object-heavy JSON

13. **SIMD-style operations**
    - Process multiple whitespace characters at once
    - Use buffer comparison operations where available
    - Estimated improvement: 5-10%

14. **Add caching layer**
    - Cache recently decoded JSON strings
    - Useful for repeated parsing of same data
    - Estimated improvement: 90%+ for cache hits

## Phase 5: Testing & Validation
**Priority: High | Impact: Critical**

15. **Expand test coverage**
    - Add more edge cases (very large numbers, deep nesting, etc.)
    - Add malformed JSON tests
    - Add encoding round-trip tests for all types

16. **Create comprehensive benchmarks**
    - Test with real-world JSON samples
    - Compare against baseline
    - Profile hot paths

## Implementation Order

**Sprint 1** (Immediate wins):
- Phase 1: Code cleanup (items 1-3)
- Phase 2: Hex digit optimization (item 4)
- Phase 2: Token parsing optimization (item 8)

**Sprint 2** (Major improvements):
- Phase 2: String parsing optimization (item 5)
- Phase 2: Number parsing optimization (item 6)
- Phase 3: String encoding optimization (item 9)

**Sprint 3** (Collection handling):
- Phase 2: Parse state caching (item 7)
- Phase 3: Collection encoding optimization (item 10)
- Phase 3: ASCII string fast path (item 11)

**Sprint 4** (Advanced features):
- Phase 4: String interning (item 12)
- Phase 4: SIMD operations (item 13)
- Phase 5: Comprehensive testing (items 15-16)

## Expected Overall Improvement
- Decoding: **30-50% faster**
- Encoding: **40-60% faster**
- Memory usage: **20-30% reduction**

## Success Metrics
- All existing tests pass
- Performance improvement ≥30% on benchmark suite
- No increase in memory usage
- Code size reduction ≥10%
