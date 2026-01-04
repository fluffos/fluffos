/*
 * FluffOS Extension Package - Production Edition v6.3 (Ultra Optimized)
 * 修复: read_json 严重的 Use-After-Free 隐患
 * 修复: 排序算法完全确定性 (Fully Deterministic Sorting)
 * 修复: compute_weight 使用 INFINITY 替代魔法数字
 * 优化: 循环检查逻辑简化
 * 优化: 内存管理的 RAII 封装
 * 优化: 添加数据截断警告机制
 */

#include "base/package_api.h"
#include "vm/internal/base/mapping.h"
#include "vm/internal/base/array.h"
#include "vm/internal/base/object.h"
#include "packages/core/file.h"
#include "yyjson.h"

#include <vector>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstring>
#include <climits>
#include <cstdlib>
#include <cmath>
#include <memory>

// =========================================================================
// 编译期优化宏
// =========================================================================

#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x)   __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
#endif

// =========================================================================
// 配置常量
// =========================================================================

#define MAX_JSON_DEPTH 128
#define MAX_JSON_FILE_SIZE (256 * 1024 * 1024)
#define MAX_JSON_STRING_LENGTH (64 * 1024 * 1024)
#define MAX_JSON_ARRAY_SIZE 10000000
#define MAX_JSON_OBJECT_SIZE 5000000
#define CIRCULAR_CHECK_THRESHOLD 24

// 浮点数比较精度
#define FLOAT_EPSILON 1e-9

// =========================================================================
// 工具类 (RAII)
// =========================================================================

struct ScopedYYDoc {
    yyjson_doc* doc;
    explicit ScopedYYDoc(yyjson_doc* d) : doc(d) {}
    ~ScopedYYDoc() { if (doc) yyjson_doc_free(doc); }
    ScopedYYDoc(const ScopedYYDoc&) = delete;
    ScopedYYDoc& operator=(const ScopedYYDoc&) = delete;
};

struct ScopedYYMutDoc {
    yyjson_mut_doc* doc;
    explicit ScopedYYMutDoc(yyjson_mut_doc* d) : doc(d) {}
    ~ScopedYYMutDoc() { if (doc) yyjson_mut_doc_free(doc); }
    ScopedYYMutDoc(const ScopedYYMutDoc&) = delete;
    ScopedYYMutDoc& operator=(const ScopedYYMutDoc&) = delete;
};

// 自动管理 malloc 内存，防止泄漏
struct AutoBuffer {
    char* ptr;
    explicit AutoBuffer(size_t size) { ptr = static_cast<char*>(malloc(size)); }
    ~AutoBuffer() { if (ptr) free(ptr); }
    bool valid() const { return ptr != nullptr; }
    AutoBuffer(const AutoBuffer&) = delete;
    AutoBuffer& operator=(const AutoBuffer&) = delete;
};

// 自动管理 FILE*，防止泄漏
struct ScopedFILE {
    FILE* fp;
    explicit ScopedFILE(FILE* f) : fp(f) {}
    ~ScopedFILE() { if (fp) fclose(fp); }
    bool valid() const { return fp != nullptr; }
    FILE* get() const { return fp; }
    ScopedFILE(const ScopedFILE&) = delete;
    ScopedFILE& operator=(const ScopedFILE&) = delete;
};

// 快速整数转字符串（优化版本，使用栈对齐）
static inline int fast_i64toa(int64_t value, char* buffer) {
    if (UNLIKELY(value == 0)) { buffer[0] = '0'; buffer[1] = '\0'; return 1; }

    // 处理 LLONG_MIN 的特殊情况，防止取反溢出
    if (UNLIKELY(value == LLONG_MIN)) {
        strcpy(buffer, "-9223372036854775808");
        return 20;
    }

    alignas(8) char temp[24];  // 对齐优化
    char* p = temp;
    uint64_t uval = (value < 0) ? static_cast<uint64_t>(-value) : static_cast<uint64_t>(value);

    while (uval > 0) { *p++ = static_cast<char>('0' + (uval % 10)); uval /= 10; }

    int len = 0;
    if (value < 0) buffer[len++] = '-';
    while (p > temp) { buffer[len++] = *--p; }
    buffer[len] = '\0';
    return len;
}

// 循环引用检测器（优化版本）
class CircularChecker {
private:
    std::vector<void*> stack_;
    std::unordered_set<void*> deep_set_;

public:
    CircularChecker() { stack_.reserve(64); }

    inline bool contains(void* ptr, int depth) const {
        // 优先检查栈（热路径，最近的引用最可能形成循环）
        // 反向查找，通常循环引用发生在最近几层
        for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
            if (*it == ptr) return true;
        }

        // 深度超过阈值才检查哈希集合
        if (depth >= CIRCULAR_CHECK_THRESHOLD) {
            return deep_set_.find(ptr) != deep_set_.end();
        }

        return false;
    }

    inline void insert(void* ptr, int depth) {
        stack_.push_back(ptr);
        if (depth >= CIRCULAR_CHECK_THRESHOLD) deep_set_.insert(ptr);
    }

    inline void remove(void* ptr, int depth) {
        stack_.pop_back();
        if (depth >= CIRCULAR_CHECK_THRESHOLD) deep_set_.erase(ptr);
    }
};

// =========================================================================
// JSON 核心实现
// =========================================================================

// 导出函数：供 Gateway 包使用
yyjson_mut_val* svalue_to_json_impl(yyjson_mut_doc* doc, svalue_t* sv,
                                             CircularChecker* checker, int depth) {
    if (UNLIKELY(depth > MAX_JSON_DEPTH)) return yyjson_mut_str(doc, "<error: max depth reached>");

    switch (sv->type) {
        case T_NUMBER: return yyjson_mut_int(doc, sv->u.number);
        case T_REAL:   return yyjson_mut_real(doc, sv->u.real);
        case T_STRING: return yyjson_mut_str(doc, sv->u.string);
        case T_ARRAY: {
            array_t* arr = sv->u.arr;
            int size = arr->size;
            if (UNLIKELY(checker->contains(arr, depth))) return yyjson_mut_str(doc, "<circular_ref_array>");

            checker->insert(arr, depth);
            yyjson_mut_val* json_arr = yyjson_mut_arr(doc);
            for (int i = 0; i < size; i++) {
                yyjson_mut_arr_append(json_arr, svalue_to_json_impl(doc, &arr->item[i], checker, depth + 1));
            }
            checker->remove(arr, depth);
            return json_arr;
        }
        case T_MAPPING: {
            mapping_t* map = sv->u.map;
            if (UNLIKELY(checker->contains(map, depth))) return yyjson_mut_str(doc, "<circular_ref_mapping>");

            checker->insert(map, depth);
            yyjson_mut_val* json_obj = yyjson_mut_obj(doc);
            char num_buf[32];
            int obj_count = 0;

            // 🔧 修复点：计算正确的桶数量 (table_size 是掩码，所以要 +1)
            int num_buckets = map->table_size + 1;

            // 🔧 修复点：使用 num_buckets 作为循环边界
            for (int i = 0; i < num_buckets; i++) {
                for (mapping_node_t *elt = map->table[i]; elt; elt = elt->next) {
                    if (UNLIKELY(++obj_count > MAX_JSON_OBJECT_SIZE)) {
                        #ifdef DEBUG
                        debug_message("json_encode: mapping truncated at %d entries\n", MAX_JSON_OBJECT_SIZE);
                        #endif
                        checker->remove(map, depth);
                        return json_obj; // 保护性截断
                    }
                    svalue_t *key = &elt->values[0];
                    svalue_t *val = &elt->values[1];

                    if (key->type == T_STRING) {
                        yyjson_mut_obj_add(json_obj, yyjson_mut_str(doc, key->u.string),
                            svalue_to_json_impl(doc, val, checker, depth + 1));
                    } else if (key->type == T_NUMBER) {
                        int len = fast_i64toa(static_cast<int64_t>(key->u.number), num_buf);
                        yyjson_mut_obj_add(json_obj, yyjson_mut_strn(doc, num_buf, static_cast<size_t>(len)),
                            svalue_to_json_impl(doc, val, checker, depth + 1));
                    }
                }
            }
            checker->remove(map, depth);
            return json_obj;
        }
        case T_OBJECT:
            if (sv->u.ob && !(sv->u.ob->flags & O_DESTRUCTED)) return yyjson_mut_str(doc, sv->u.ob->obname);
            return yyjson_mut_null(doc);
        default: return yyjson_mut_null(doc);
    }
}

// 导出函数：供 Gateway 包使用
void json_to_svalue(yyjson_val* val, svalue_t* out, int depth) {
    out->type = T_NUMBER; out->subtype = 0; out->u.number = 0;
    if (UNLIKELY(!val || depth > MAX_JSON_DEPTH)) return;

    switch (yyjson_get_type(val)) {
        case YYJSON_TYPE_NULL: break;
        case YYJSON_TYPE_BOOL: out->u.number = yyjson_get_bool(val) ? 1 : 0; break;
        case YYJSON_TYPE_NUM:
            if (yyjson_is_real(val)) { out->type = T_REAL; out->u.real = yyjson_get_real(val); }
            else { out->u.number = static_cast<long>(yyjson_get_int(val)); }
            break;
        case YYJSON_TYPE_STR: {
            size_t len = yyjson_get_len(val);
            // 增加对空字符串的快速处理
            if (len == 0) {
                 out->type = T_STRING; out->subtype = STRING_MALLOC;
                 out->u.string = string_copy("", "json_empty");
                 break;
            }
            if (UNLIKELY(len > MAX_JSON_STRING_LENGTH)) {
                out->type = T_STRING; out->subtype = STRING_MALLOC;
                out->u.string = string_copy("", "json_decode_overflow");
                break;
            }
            out->type = T_STRING; out->subtype = STRING_MALLOC;
            out->u.string = string_copy(yyjson_get_str(val), "json_decode");
            break;
        }
        case YYJSON_TYPE_ARR: {
            size_t count = yyjson_arr_size(val);
            if (UNLIKELY(count > MAX_JSON_ARRAY_SIZE)) {
                #ifdef DEBUG
                debug_message("json_decode: array truncated from %zu to %d elements\n", count, MAX_JSON_ARRAY_SIZE);
                #endif
                count = MAX_JSON_ARRAY_SIZE;
            }
            array_t* lpc_arr = allocate_array(static_cast<int>(count));
            out->type = T_ARRAY; out->u.arr = lpc_arr;

            yyjson_val* item; size_t idx, max;
            yyjson_arr_foreach(val, idx, max, item) {
                if (idx >= count) break;
                json_to_svalue(item, &lpc_arr->item[idx], depth + 1);
            }
            break;
        }
        case YYJSON_TYPE_OBJ: {
            size_t count = yyjson_obj_size(val);
            if (UNLIKELY(count > MAX_JSON_OBJECT_SIZE)) {
                #ifdef DEBUG
                debug_message("json_decode: object truncated from %zu to %d entries\n", count, MAX_JSON_OBJECT_SIZE);
                #endif
                count = MAX_JSON_OBJECT_SIZE;
            }
            mapping_t* lpc_map = allocate_mapping(static_cast<int>(count));
            out->type = T_MAPPING; out->u.map = lpc_map;

            yyjson_val *key, *ele; size_t idx, max; int inserted = 0;
            yyjson_obj_foreach(val, idx, max, key, ele) {
                if (static_cast<size_t>(inserted) >= count) break;

                svalue_t key_sv; key_sv.type = T_STRING; key_sv.subtype = STRING_MALLOC;
                key_sv.u.string = string_copy(yyjson_get_str(key), "json_key");

                svalue_t* dest = find_for_insert(lpc_map, &key_sv, 1);

                if (dest) {
                    json_to_svalue(ele, dest, depth + 1);
                    inserted++;
                }
                free_string_svalue(&key_sv);
            }
            break;
        }
    }
}

// =========================================================================
// EFUNS: JSON
// =========================================================================

void f_json_encode(void) {
    svalue_t* arg = sp;
    yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
    if (UNLIKELY(!doc)) { pop_n_elems(1); push_number(0); return; }
    ScopedYYMutDoc doc_guard(doc);

    CircularChecker checker;
    yyjson_mut_val* root = svalue_to_json_impl(doc, arg, &checker, 0);
    yyjson_mut_doc_set_root(doc, root);

    size_t len;
    char* json_str = yyjson_mut_write(doc, 0, &len);
    if (UNLIKELY(!json_str)) { pop_n_elems(1); push_number(0); return; }

    pop_n_elems(1);
    copy_and_push_string(json_str);
    free(json_str);
}

void f_json_decode(void) {
    const char* str = sp->u.string;
    if (UNLIKELY(!str || !*str)) { pop_n_elems(1); push_number(0); return; }

    yyjson_read_err err;
    // 注意: flags=0 时 yyjson 不会修改输入缓冲区，const_cast 是安全的
    // 使用 SVALUE_STRLEN 获取字符串长度，避免 strlen 的潜在问题
    size_t str_len = SVALUE_STRLEN(sp);
    yyjson_doc* doc = yyjson_read_opts(const_cast<char*>(str), str_len, 0, NULL, &err);

    if (UNLIKELY(!doc)) {
        #ifdef DEBUG
        debug_message("json_decode failed: %s at pos %zu\n", err.msg, err.pos);
        #endif
        pop_n_elems(1); push_number(0); return;
    }

    ScopedYYDoc doc_guard(doc);
    svalue_t result;
    json_to_svalue(yyjson_doc_get_root(doc), &result, 0);

    pop_n_elems(1);
    sp++;
    *sp = result;
}

void f_read_json(void) {
    const char* filename = sp->u.string;
    const char* real_path = check_valid_path(filename, current_object, "read_json", 0);
    if (UNLIKELY(!real_path)) { pop_n_elems(1); push_number(0); return; }

    ScopedFILE fp_guard(fopen(real_path, "rb"));
    if (UNLIKELY(!fp_guard.valid())) { pop_n_elems(1); push_number(0); return; }
    FILE* fp = fp_guard.get();

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (UNLIKELY(fsize <= 0 || fsize > MAX_JSON_FILE_SIZE)) {
        pop_n_elems(1); push_number(0); return;
    }

    // 使用 AutoBuffer 确保内存释放，特别是在中途返回时
    size_t buf_size = static_cast<size_t>(fsize) + YYJSON_PADDING_SIZE;
    AutoBuffer buffer(buf_size);
    if (UNLIKELY(!buffer.valid())) {
        pop_n_elems(1); push_number(0); return;
    }

    size_t read_size = fread(buffer.ptr, 1, static_cast<size_t>(fsize), fp);
    if (UNLIKELY(read_size != static_cast<size_t>(fsize))) {
        pop_n_elems(1); push_number(0); return;
    }

    yyjson_read_err err;
    // 使用 INSITU 模式提高性能
    // 注意: buffer 必须存活至 doc 销毁后（AutoBuffer 的析构在外层作用域）
    yyjson_doc* doc = yyjson_read_opts(buffer.ptr, static_cast<size_t>(fsize),
                                       YYJSON_READ_INSITU,
                                       NULL, &err);
    if (UNLIKELY(!doc)) {
        #ifdef DEBUG
        debug_message("read_json failed: %s at pos %zu\n", err.msg, err.pos);
        #endif
        pop_n_elems(1); push_number(0); return;
    }

    // 作用域控制：确保 doc 在 buffer 之前销毁
    // json_to_svalue 会复制所有字符串到 LPC 字符串池，不持有 buffer 引用
    {
        ScopedYYDoc doc_guard(doc);
        svalue_t result;
        json_to_svalue(yyjson_doc_get_root(doc), &result, 0);

        pop_n_elems(1);
        sp++;
        *sp = result;
    }
    // doc_guard 析构 -> yyjson_doc_free(doc)
    // fp_guard 析构 -> fclose(fp)
    // 离开函数 -> AutoBuffer 析构 -> free(buffer.ptr)
}

void f_write_json(void) {
    svalue_t* data = sp;
    const char* filename = (sp - 1)->u.string;
    const char* real_path = check_valid_path(filename, current_object, "write_json", 1);
    if (UNLIKELY(!real_path)) { pop_n_elems(2); push_number(0); return; }

    yyjson_mut_doc* doc = yyjson_mut_doc_new(NULL);
    if (UNLIKELY(!doc)) { pop_n_elems(2); push_number(0); return; }
    ScopedYYMutDoc doc_guard(doc);

    CircularChecker checker;
    yyjson_mut_val* root = svalue_to_json_impl(doc, data, &checker, 0);
    yyjson_mut_doc_set_root(doc, root);

    ScopedFILE fp_guard(fopen(real_path, "wb"));
    if (UNLIKELY(!fp_guard.valid())) { pop_n_elems(2); push_number(0); return; }

    yyjson_write_err err;
    bool success = yyjson_mut_write_fp(fp_guard.get(), doc, 0, NULL, &err);

    if (UNLIKELY(!success)) {
        #ifdef DEBUG
        debug_message("write_json failed: %s\n", err.msg);
        #endif
    }
    // fp_guard 析构时自动关闭文件

    pop_n_elems(2);
    push_number(success ? 1 : 0);
}


// =========================================================================
// 快速排序 (完全确定性版本)
// =========================================================================

struct SortEntry {
    svalue_t* key;
    double weight;
};

// 计算权重（使用 -INFINITY 替代魔法数字）
static inline double compute_weight(const svalue_t* v) {
    switch (v->type) {
        case T_NUMBER: return static_cast<double>(v->u.number);
        case T_REAL:   return v->u.real;
        case T_STRING: {
            char* end;
            double val = strtod(v->u.string, &end);
            // 确保至少解析了一个字符
            return (end != v->u.string) ? val : -INFINITY;
        }
        default: return -INFINITY;
    }
}

// svalue 内容比较（用于确定性 tie-breaking）
static inline int compare_svalue_content(const svalue_t* a, const svalue_t* b) {
    if (a->type != b->type) return a->type - b->type;

    switch (a->type) {
        case T_NUMBER:
            if (a->u.number < b->u.number) return -1;
            if (a->u.number > b->u.number) return 1;
            return 0;
        case T_REAL:
            if (a->u.real < b->u.real) return -1;
            if (a->u.real > b->u.real) return 1;
            return 0;
        case T_STRING: {
            int cmp = strcmp(a->u.string, b->u.string);
            // 标准化返回值为 -1/0/1
            if (cmp < 0) return -1;
            if (cmp > 0) return 1;
            // 字符串内容相同，比较地址确保稳定性
            return (a < b) ? -1 : ((a > b) ? 1 : 0);
        }
        default:
            // 其他类型按指针地址排序（保证稳定性）
            return (a < b) ? -1 : ((a > b) ? 1 : 0);
    }
}

// 确定性升序比较
bool compare_sort_asc(const SortEntry& a, const SortEntry& b) {
    // 先比较权重（严格的数值比较，避免 epsilon 导致的传递性问题）
    if (a.weight < b.weight) return true;
    if (a.weight > b.weight) return false;

    // 权重完全相等时，比较 svalue 内容以保证确定性
    return compare_svalue_content(a.key, b.key) < 0;
}

// 确定性降序比较
bool compare_sort_desc(const SortEntry& a, const SortEntry& b) {
    // 先比较权重（严格的数值比较）
    if (a.weight > b.weight) return true;
    if (a.weight < b.weight) return false;

    // 权重完全相等时，比较 svalue 内容
    return compare_svalue_content(a.key, b.key) > 0;
}

void f_sort_mapping(void) {
    // 支持可选参数：sort_mapping(mapping) 或 sort_mapping(mapping, flag)
    // flag 默认为 0（升序）
    int num_args = st_num_arg;
    int flag = 0;  // 默认升序
    mapping_t* m;

    if (num_args == 2) {
        // 两个参数：mapping 和 flag
        flag = (sp)->u.number;
        m = (sp - 1)->u.map;
    } else {
        // 一个参数：只有 mapping，flag 使用默认值 0
        m = (sp)->u.map;
    }

    std::vector<SortEntry> entries;
    entries.reserve(m->count);

    // 关键修复：table_size 是哈希掩码，实际桶数量是 table_size + 1
    // 例如 table_size=15 表示有 16 个桶（索引 0-15）
    int num_buckets = m->table_size + 1;

    for (int i = 0; i < num_buckets; i++) {
        for (mapping_node_t* elt = m->table[i]; elt; elt = elt->next) {
            SortEntry entry;
            entry.key = &elt->values[0];
            entry.weight = compute_weight(&elt->values[1]);
            entries.push_back(entry);
        }
    }

    if (flag == 1) std::sort(entries.begin(), entries.end(), compare_sort_desc);
    else std::sort(entries.begin(), entries.end(), compare_sort_asc);

    array_t* result = allocate_array(static_cast<int>(entries.size()));
    for (size_t i = 0; i < entries.size(); ++i) {
        assign_svalue_no_free(&result->item[i], entries[i].key);
    }

    pop_n_elems(num_args);
    push_refed_array(result);
}

// -------------------------------------------------------------------------

struct IntEntry {
    svalue_t* key_ptr;
    int64_t val;
};

// 确定性升序比较（整数版本）
bool compare_int_asc(const IntEntry& a, const IntEntry& b) {
    if (a.val != b.val) return a.val < b.val;

    // 值相同时比较 key 内容
    return compare_svalue_content(a.key_ptr, b.key_ptr) < 0;
}

// 确定性降序比较（整数版本）
bool compare_int_desc(const IntEntry& a, const IntEntry& b) {
    if (a.val != b.val) return a.val > b.val;

    return compare_svalue_content(a.key_ptr, b.key_ptr) > 0;
}

void f_sort_mapping_int(void) {
    // 支持可选参数：sort_mapping_int(mapping) 或 sort_mapping_int(mapping, flag)
    // flag 默认为 0（升序）
    int num_args = st_num_arg;
    int flag = 0;  // 默认升序
    mapping_t* m;

    if (num_args == 2) {
        // 两个参数：mapping 和 flag
        flag = (sp)->u.number;
        m = (sp - 1)->u.map;
    } else {
        // 一个参数：只有 mapping，flag 使用默认值 0
        m = (sp)->u.map;
    }

    std::vector<IntEntry> entries;
    entries.reserve(m->count);

    // 关键修复：table_size 是哈希掩码，实际桶数量是 table_size + 1
    int num_buckets = m->table_size + 1;

    for (int i = 0; i < num_buckets; i++) {
        for (mapping_node_t* elt = m->table[i]; elt; elt = elt->next) {
            IntEntry entry;
            entry.key_ptr = &elt->values[0];
            if (elt->values[1].type == T_NUMBER) {
                entry.val = elt->values[1].u.number;
            } else {
                entry.val = 0;
            }
            entries.push_back(entry);
        }
    }

    if (flag == 1) std::sort(entries.begin(), entries.end(), compare_int_desc);
    else std::sort(entries.begin(), entries.end(), compare_int_asc);

    array_t* result = allocate_array(static_cast<int>(entries.size()));
    for (size_t i = 0; i < entries.size(); ++i) {
        assign_svalue_no_free(&result->item[i], entries[i].key_ptr);
    }

    pop_n_elems(num_args);
    push_refed_array(result);
}

// =========================================================================
// 导出函数：供 Gateway 包使用
// =========================================================================

int svalue_to_json_string(svalue_t* sv, char** out_json_str, size_t* out_len) {
    if (!sv || !out_json_str || !out_len) return 0;

    yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
    if (!doc) return 0;
    ScopedYYMutDoc doc_guard(doc);

    CircularChecker checker;
    yyjson_mut_val* root = svalue_to_json_impl(doc, sv, &checker, 0);
    if (!root) return 0;

    yyjson_mut_doc_set_root(doc, root);

    size_t len = 0;
    char* json_str = yyjson_mut_write(doc, 0, &len);
    if (!json_str) return 0;

    *out_json_str = json_str;
    *out_len = len;
    return 1;
}

// =========================================================================
// Gateway Output 消息包构建函数 (供 gateway_send_to_session 使用)
// =========================================================================

/**
 * 构建 Gateway Output 消息包
 * 格式: {"type":"output", "cid":"SESSION_ID", "data":"DATA_STRING"}
 * 用于系统内部文本消息发送 (如 tell_object, write)
 * 注意：data 是字符串，不是 JSON 对象
 */
extern "C" int build_gateway_output_packet(const char* cid, const char* data, char** out_json_str, size_t* out_len) {
    if (!cid || !data || !out_json_str || !out_len) return 0;

    yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);
    if (!doc) return 0;
    ScopedYYMutDoc doc_guard(doc);

    yyjson_mut_val* root = yyjson_mut_obj(doc);
    yyjson_mut_doc_set_root(doc, root);

    yyjson_mut_obj_add_str(doc, root, "type", "output");
    yyjson_mut_obj_add_str(doc, root, "cid", cid);
    yyjson_mut_obj_add_str(doc, root, "data", data);

    size_t len = 0;
    char* json_str = yyjson_mut_write(doc, 0, &len);
    if (!json_str) return 0;

    *out_json_str = json_str;
    *out_len = len;
    return 1;
}
