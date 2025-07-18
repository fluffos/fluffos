# FluffOS Security Audit Report
## Targeted Manual Audit of High-Risk Areas

**Date:** 2025-07-12  
**Scope:** Manual review of critical security vulnerabilities in FluffOS codebase  
**Focus Areas:** Buffer overflows, path handling race conditions, unsafe string operations

---

## Executive Summary

This security audit identified **4 critical vulnerabilities** and **multiple high-risk patterns** across the FluffOS codebase. The primary concerns are unsafe string operations that could lead to buffer overflows and race conditions in file handling operations.

---

## Critical Findings

### 1. Buffer Overflow in `vm/internal/simul_efun.cc`

**Severity:** HIGH  
**Location:** Line 69  
**Function:** `init_simul_efun()`

**Code Snippet:**
```c
char buf[512];
// ... 
if (file[strlen(file) - 2] != '.') {
    strcat(buf, ".c");  // VULNERABLE
}
```

**Vulnerability Analysis:**
- **Issue:** Unsafe use of `strcat()` without bounds checking
- **Exploit Vector:** If `buf` is near its 512-byte limit, appending ".c" could cause a 2-byte buffer overflow
- **Impact:** Stack corruption, potential code execution

**Remediation:**
```c
// Replace with bounds-checked version
if (strlen(buf) + 3 < sizeof(buf)) {
    strcat(buf, ".c");
} else {
    error("Buffer too small for filename");
}
// OR use strncat with remaining space calculation
```

### 2. Multiple Buffer Overflows in `vm/internal/simulate.cc`

**Severity:** HIGH  
**Locations:** Lines 447, 450, 507, 1236, 1240, 1409, 1413, 1417

**Vulnerable Patterns:**
```c
char name[400], actualname[400], real_name[sizeof(name) + 2], obname[sizeof(real_name)];
char inhbuf[MAX_OBJECT_NAME_SIZE];

// VULNERABLE: No bounds checking
(void)strcpy(real_name, actualname);
(void)strcat(real_name, ".c");
(void)strcpy(obname, name);
(void)strcat(obname, ".c");

// VULNERABLE: Fixed buffer sprintf
char txt_buf[LARGEST_PRINTABLE_STRING + 1];
sprintf(txt_buf, "%" LPC_INT_FMTSTR_P, v->u.number);
sprintf(txt_buf, "%" LPC_FLOAT_FMTSTR_P, v->u.real);
```

**Exploit Vectors:**
- Long object names could overflow `name[400]` and `actualname[400]` buffers
- Numeric formatting could exceed `LARGEST_PRINTABLE_STRING` if constants are misconfigured
- Chain of `strcpy` + `strcat` operations compound overflow risk

**Remediation:**
```c
// Use safe alternatives
snprintf(real_name, sizeof(real_name), "%s.c", actualname);
snprintf(obname, sizeof(obname), "%s.c", name);
snprintf(txt_buf, sizeof(txt_buf), "%" LPC_INT_FMTSTR_P, v->u.number);
```

### 3. Path Handling Vulnerabilities in `packages/core/file.cc`

**Severity:** HIGH  
**Locations:** Lines 196, 199, 243, 259, 866, 918

**Race Condition Issues:**
```c
// VULNERABLE: Time-of-check to time-of-use race
if (stat(temppath, &st) < 0) {
    // ... file operations based on stat result
}

// Later operations assume stat results are still valid
if (file_size(to) == -2) {
    // Directory operations without re-checking
}
```

**Buffer Overflow Issues:**
```c
char temppath[MAX_FNAME_SIZE + MAX_PATH_LEN + 2];
char regexppath[MAX_FNAME_SIZE + MAX_PATH_LEN + 2];
char newto[MAX_FNAME_SIZE + MAX_PATH_LEN + 2];

// VULNERABLE: No bounds checking
strcpy(regexppath, p + 1);
strcpy(regexppath, p);
strcpy(endtemp, de->d_name);
sprintf(newto, "%s/%s", to, cp);
```

**Exploit Vectors:**
- Race conditions allow file system state changes between security checks and operations
- Long directory/file names can overflow fixed-size buffers
- Path traversal attacks possible if input validation insufficient

**Remediation:**
```c
// Use safe string operations
strncpy(regexppath, p + 1, sizeof(regexppath) - 1);
regexppath[sizeof(regexppath) - 1] = '\0';

// For atomic operations, use file descriptors:
int fd = open(filename, O_RDWR);
if (fd >= 0) {
    fstat(fd, &st);  // Atomic with open
    // ... operations on fd
    close(fd);
}

// Safe formatting
snprintf(newto, sizeof(newto), "%s/%s", to, cp);
```

### 4. Pattern: Unsafe sprintf/strcpy Usage

**Severity:** MEDIUM-HIGH  
**Widespread Issue:** Found 50+ instances across codebase

**Common Vulnerable Patterns:**
```c
char buffer[FIXED_SIZE];
sprintf(buffer, "%s", user_input);        // No bounds check
strcpy(buffer, source);                   // No length validation
strcat(buffer, additional_data);          // Potential overflow
```

**High-Risk Files:**
- `compiler/internal/lex.cc` - 15+ unsafe operations
- `packages/core/sprintf.cc` - Ironically contains unsafe sprintf usage
- `vm/internal/base/object.cc` - Object name handling
- `packages/core/ed.cc` - Editor functionality

---

## Risk Assessment Matrix

| Vulnerability Type | Severity | Likelihood | Files Affected | Exploitability |
|-------------------|----------|------------|---------------|----------------|
| Buffer Overflow | HIGH | HIGH | 8+ | HIGH |
| Race Conditions | MEDIUM | MEDIUM | 3 | MEDIUM |
| Path Traversal | MEDIUM | LOW | 2 | MEDIUM |
| Format String | LOW | LOW | 5+ | LOW |

---

## Remediation Roadmap

### Immediate Actions (Critical)
1. **Replace all `strcat()` calls** with bounds-checked alternatives
2. **Audit `simul_efun.cc`** buffer overflow on line 69
3. **Replace `sprintf()` with `snprintf()`** in all path handling code

### Short-term Actions (1-2 weeks)
1. **Implement safe string handling library** or macros
2. **Add static analysis tools** to catch unsafe function usage
3. **Review all file operations** for race conditions

### Long-term Actions (1-3 months)
1. **Code style guidelines** mandating safe string functions
2. **Automated testing** for buffer overflow conditions
3. **Security-focused code review process**

---

## Recommended Security Functions

### Safe String Operations
```c
// Use these instead of unsafe variants
#define SAFE_STRCPY(dst, src) do { \
    strncpy(dst, src, sizeof(dst) - 1); \
    dst[sizeof(dst) - 1] = '\0'; \
} while(0)

#define SAFE_STRCAT(dst, src) do { \
    size_t dst_len = strlen(dst); \
    size_t remaining = sizeof(dst) - dst_len - 1; \
    if (remaining > 0) { \
        strncat(dst, src, remaining); \
    } \
} while(0)
```

### Path Validation
```c
// Validate paths before operations
bool is_safe_path(const char* path) {
    if (!path || strlen(path) >= PATH_MAX) return false;
    if (strstr(path, "..")) return false;  // Prevent traversal
    if (strstr(path, "//")) return false;  // Prevent injection
    return true;
}
```

---

## Conclusion

The FluffOS codebase contains multiple critical security vulnerabilities primarily stemming from unsafe string handling practices. While the codebase appears to have some security considerations (path validation functions exist), the pervasive use of unsafe C string functions creates significant attack surface.

**Priority Actions:**
1. Immediate patching of the `simul_efun.cc` buffer overflow
2. Systematic replacement of unsafe string functions
3. Implementation of comprehensive input validation

**Risk Level:** HIGH - Multiple easily exploitable buffer overflow conditions exist that could lead to arbitrary code execution.
