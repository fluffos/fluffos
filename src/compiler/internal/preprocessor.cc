// preprocessor.cc — standalone LPC C-preprocessor stage
//
// Design: operate entirely on std::string, parsing with std::string_view.
//   - Slurps input via LexStream into a std::string
//   - Walks with a size_t cursor
//   - Uses std::string_view to avoid copying substrings
//   - Evaluates #if expressions using a clean IfExprParser class
//   - All state lives in Impl — zero global variables
//
// To use after preprocessing:
//   std::string result = LpcPreprocessor(stream, file).preprocess();
//   std::istringstream ss(result);
//   start_new_file(std::make_unique<IStreamLexStream>(ss));

#include "base/std.h"

#include "compiler/internal/preprocessor.h"
#include "vm/internal/base/number.h"
#include "compiler/internal/lexer_utils.h"
#include "compiler/internal/lex.h"
#include "vm/vm.h"
#include "compiler.h"
#include "base/internal/file.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <fcntl.h>
#include <unistd.h>

#ifdef _WIN32
#include <io.h>
#endif

extern const char *check_valid_path(const char *, object_t *, const char *const, int);

namespace {

std::string normalize_filename(const char* filename) {
    if (!filename) return "/unknown";
    if (filename[0] == '/') return filename;
    return "/" + std::string(filename);
}

// ---------------------------------------------------------------------------
// PpMacro — one macro entry
// ---------------------------------------------------------------------------
struct PpMacro {
    bool is_function_like = false;
    bool is_predefined = false;
    std::vector<std::string> params;  // parameter names (function-like only)
    std::string body;                 // replacement body
};

// Trim leading and trailing ASCII spaces, tabs, and carriage returns
std::string_view trim(std::string_view s) {
    size_t a = s.find_first_not_of(" \t\r");
    if (a == std::string_view::npos) return "";
    size_t b = s.find_last_not_of(" \t\r");
    return s.substr(a, b - a + 1);
}

// Strip // and /* */ comments from a directive argument string.
std::string strip_directive_comments(std::string_view s) {
    std::string r;
    r.reserve(s.size());
    size_t i = 0;
    while (i < s.size()) {
        if (s[i] == '"' || s[i] == '\'') {
            char q = s[i++];
            r += q;
            while (i < s.size() && s[i] != q) {
                if (s[i] == '\\') { r += s[i++]; }
                if (i < s.size()) r += s[i++];
            }
            if (i < s.size()) r += s[i++];
        } else if (i + 1 < s.size() && s[i] == '/' && s[i+1] == '/') {
            break; // rest is a comment
        } else if (i + 1 < s.size() && s[i] == '/' && s[i+1] == '*') {
            i += 2;
            while (i + 1 < s.size() && (s[i] != '*' || s[i+1] != '/')) i++;
            if (i + 1 < s.size()) i += 2;
        } else {
            r += s[i++];
        }
    }
    return r;
}

// Helper to convert a string to a C/LPC string literal (escaping quotes/backslashes)
std::string stringize(std::string_view s) {
    std::string r = "\"";
    for (char c : s) {
        if (c == '"' || c == '\\') {
            r += '\\';
        }
        r += c;
    }
    r += '"';
    return r;
}

// ---------------------------------------------------------------------------
// IfExprParser — parses and evaluates #if / #elif integer expressions
// ---------------------------------------------------------------------------
class IfExprParser {
public:
    explicit IfExprParser(std::string_view expr) : expr_(expr), pos_(0) {}

    long parse() { return top(); }

    [[nodiscard]] bool has_error() const { return !error_.empty(); }
    [[nodiscard]] const std::string& error() const { return error_; }

    // True when all non-whitespace input has been consumed.
    [[nodiscard]] bool at_end_of_expr() const {
        size_t p = pos_;
        while (p < expr_.size() && std::isspace(static_cast<unsigned char>(expr_[p]))) p++;
        return p >= expr_.size();
    }

private:
    std::string_view expr_;
    size_t pos_;
    std::string error_;

    bool at_end() const { return pos_ >= expr_.size(); }

    char peek() const { return at_end() ? '\0' : expr_[pos_]; }

    char advance() { return at_end() ? '\0' : expr_[pos_++]; }

    void skip_whitespace() {
        while (!at_end() && std::isspace(static_cast<unsigned char>(expr_[pos_])))
            pos_++;
    }

    void set_error(std::string msg) {
        if (error_.empty()) error_ = std::move(msg);
    }

    long atom() {
        skip_whitespace();
        if (at_end()) return 0;

        char c = peek();

        if (c == '(') {
            advance();
            long v = top();
            skip_whitespace();
            if (peek() == ')') {
                advance();
            } else {
                set_error("bracket not paired in #if");
            }
            return v;
        }
        if (c == '!') { advance(); return !atom(); }
        if (c == '~') { advance(); return ~atom(); }
        if (c == '-') { advance(); return -atom(); }
        if (c == '+') { advance(); return atom(); }

        // Character literal: 'x', '\n', '\t', '\ooo', '\xhh'
        if (c == '\'') {
            advance(); // consume opening '\''
            long value = 0;
            if (!at_end() && peek() == '\\') {
                advance(); // consume backslash
                if (!at_end()) {
                    char esc = advance();
                    switch (esc) {
                        case 'n':  value = '\n'; break;
                        case 't':  value = '\t'; break;
                        case 'r':  value = '\r'; break;
                        case 'a':  value = '\a'; break;
                        case 'b':  value = '\b'; break;
                        case 'f':  value = '\f'; break;
                        case 'v':  value = '\v'; break;
                        case 'e':  value = '\x1b'; break;
                        case '0':  value = '\0'; break;
                        case '\\': value = '\\'; break;
                        case '\'': value = '\''; break;
                        case '"':  value = '"';  break;
                        case 'x': {
                            // Hex escape \xNN
                            long hv = 0;
                            while (!at_end() && std::isxdigit(static_cast<unsigned char>(peek()))) {
                                char h = advance();
                                hv = (hv * 16) + (std::isdigit(static_cast<unsigned char>(h))
                                    ? h - '0'
                                    : std::tolower(static_cast<unsigned char>(h)) - 'a' + 10);
                            }
                            value = hv;
                            break;
                        }
                        default:
                            if (std::isdigit(static_cast<unsigned char>(esc))) {
                                // Octal escape \ooo
                                long ov = esc - '0';
                                for (int i = 0; i < 2 && !at_end() && peek() >= '0' && peek() <= '7'; i++) {
                                    ov = (ov * 8) + (advance() - '0');
                                }
                                value = ov;
                            } else {
                                value = static_cast<unsigned char>(esc);
                            }
                    }
                }
            } else if (!at_end() && peek() != '\'') {
                value = static_cast<unsigned char>(advance());
            }
            if (!at_end() && peek() == '\'') advance(); // consume closing '\''
            return value;
        }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            size_t start = pos_;
            if (c == '0' && pos_ + 1 < expr_.size() &&
                (expr_[pos_ + 1] == 'x' || expr_[pos_ + 1] == 'X')) {
                pos_ += 2;
                while (!at_end() && std::isxdigit(static_cast<unsigned char>(expr_[pos_])))
                    pos_++;
            } else {
                while (!at_end() && std::isdigit(static_cast<unsigned char>(expr_[pos_])))
                    pos_++;
            }
            // Strip L/U suffixes before passing to stol
            while (!at_end() && (expr_[pos_] == 'L' || expr_[pos_] == 'U' ||
                                 expr_[pos_] == 'l' || expr_[pos_] == 'u'))
                pos_++;
            std::string literal(expr_.substr(start, pos_ - start));
            try { return std::stol(literal, nullptr, 0); } catch (...) { return 0; }
        }

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            while (!at_end() && (std::isalnum(static_cast<unsigned char>(expr_[pos_])) || expr_[pos_] == '_'))
                pos_++;
            return 0; // undefined identifier evaluates to 0
        }

        advance();
        return 0;
    }

    long binop(int min_prec) {
        long lhs = atom();
        for (;;) {
            skip_whitespace();
            if (at_end()) break;

            char c0 = expr_[pos_];
            char c1 = (pos_ + 1 < expr_.size()) ? expr_[pos_ + 1] : '\0';

            int prec = -1, op_len = 1, op = 0;

            if      (c0 == '|' && c1 == '|') { op = 'O'; prec = 1; op_len = 2; }
            else if (c0 == '&' && c1 == '&') { op = 'A'; prec = 2; op_len = 2; }
            else if (c0 == '=' && c1 == '=') { op = 'E'; prec = 6; op_len = 2; }
            else if (c0 == '!' && c1 == '=') { op = 'N'; prec = 6; op_len = 2; }
            else if (c0 == '<' && c1 == '=') { op = 'L'; prec = 7; op_len = 2; }
            else if (c0 == '>' && c1 == '=') { op = 'G'; prec = 7; op_len = 2; }
            else if (c0 == '<' && c1 == '<') { op = 's'; prec = 8; op_len = 2; }
            else if (c0 == '>' && c1 == '>') { op = 'S'; prec = 8; op_len = 2; }
            else if (c0 == '|') { op = '|'; prec = 3; }
            else if (c0 == '^') { op = '^'; prec = 4; }
            else if (c0 == '&') { op = '&'; prec = 5; }
            else if (c0 == '<') { op = '<'; prec = 7; }
            else if (c0 == '>') { op = '>'; prec = 7; }
            else if (c0 == '+') { op = '+'; prec = 9; }
            else if (c0 == '-') { op = '-'; prec = 9; }
            else if (c0 == '*') { op = '*'; prec = 10; }
            else if (c0 == '/') { op = '/'; prec = 10; }
            else if (c0 == '%') { op = '%'; prec = 10; }

            if (prec < min_prec) break;

            pos_ += op_len;
            long rhs = binop(prec + 1);

            switch (op) {
                case 'O': lhs = lhs || rhs; break;
                case 'A': lhs = lhs && rhs; break;
                case '|': lhs = lhs | rhs; break;
                case '^': lhs = lhs ^ rhs; break;
                case '&': lhs = lhs & rhs; break;
                case 'E': lhs = lhs == rhs; break;
                case 'N': lhs = lhs != rhs; break;
                case 'L': lhs = lhs <= rhs; break;
                case 'G': lhs = lhs >= rhs; break;
                case '<': lhs = lhs < rhs; break;
                case '>': lhs = lhs > rhs; break;
                case 's': lhs = lhs << rhs; break;
                case 'S': lhs = lhs >> rhs; break;
                case '+': lhs = lhs + rhs; break;
                case '-': lhs = lhs - rhs; break;
                case '*': lhs = lhs * rhs; break;
                case '/':
                    if (rhs == 0) { set_error("division by 0 in #if"); lhs = 0; }
                    else { lhs = lhs / rhs; }
                    break;
                case '%':
                    if (rhs == 0) { set_error("modulo by 0 in #if"); lhs = 0; }
                    else { lhs = lhs % rhs; }
                    break;
                default: break;
            }
        }
        return lhs;
    }

    // top() handles the ternary operator ?: (lowest precedence, right-associative)
    long top() {
        long cond = binop(0);
        skip_whitespace();
        if (!at_end() && peek() == '?') {
            advance(); // consume '?'
            skip_whitespace();
            long true_val = top(); // right-associative
            skip_whitespace();
            if (!at_end() && peek() == ':') {
                advance(); // consume ':'
            } else {
                set_error("'?' without ':' in #if");
            }
            skip_whitespace();
            long false_val = top();
            return cond ? true_val : false_val;
        }
        return cond;
    }
};

} // namespace

// ---------------------------------------------------------------------------
// LpcPreprocessor::Impl
// ---------------------------------------------------------------------------
struct LpcPreprocessor::Impl {
    // ── State ──────────────────────────────────────────────────────────────
    std::string src_;       // complete source text (slurped from stream)
    size_t      pos_;       // current read cursor into src_
    std::string out_;       // accumulated output
    std::string filename_;  // current filename

    std::unordered_map<std::string, PpMacro> local_defines_;
    std::unordered_map<std::string, PpMacro>& defines_;

    // Conditional-compilation stack.
    // Each entry is (currently_emitting, already_had_a_true_branch).
    struct CondState { bool emitting; bool had_true; };
    std::vector<CondState> cond_stack_;

    int current_line_;
    int include_depth_;
    std::vector<std::string>& errors_;
    std::vector<std::string> local_errors_;

    // ── Constructors ───────────────────────────────────────────────────────
    Impl(std::unique_ptr<LexStream> stream, const char* filename)
        : pos_(0), filename_(normalize_filename(filename)),
          defines_(local_defines_), current_line_(1), include_depth_(0), errors_(local_errors_) {
        char buf[8192];
        size_t n;
        while ((n = stream->read(buf, sizeof(buf))) > 0)
            src_.append(buf, n);
        stream->close();

        add_builtin_macros();
    }

    Impl(std::unique_ptr<LexStream> stream, const char* filename,
         std::unordered_map<std::string, PpMacro>& shared_defines,
         std::vector<std::string>& shared_errors,
         int include_depth = 0)
        : pos_(0), filename_(normalize_filename(filename)),
          defines_(shared_defines), current_line_(1), include_depth_(include_depth),
          errors_(shared_errors) {
        char buf[8192];
        size_t n;
        while ((n = stream->read(buf, sizeof(buf))) > 0)
            src_.append(buf, n);
        stream->close();
    }

    Impl(std::string source, const char* filename)
        : src_(std::move(source)), pos_(0), filename_(normalize_filename(filename)),
          defines_(local_defines_), current_line_(1), include_depth_(0), errors_(local_errors_) {
        add_builtin_macros();
    }

    // ── Built-in Macros ───────────────────────────────────────────────────
    void add_builtin_macros() {
        const auto& predefs = get_predefines();
        for (const auto& pair : predefs) {
            PpMacro m;
            m.is_function_like = pair.second.is_function_like;
            m.is_predefined = true;
            m.body = pair.second.body;
            defines_[pair.first] = std::move(m);
        }
        defines_["__FILE__"] = PpMacro{false, true, {}, ""};
        defines_["__LINE__"] = PpMacro{false, true, {}, ""};
        defines_["__DIR__"] = PpMacro{false, true, {}, ""};
    }

    std::string get_dir_macro() const {
        size_t last_slash = filename_.find_last_of('/');
        if (last_slash != std::string::npos) {
            return stringize(filename_.substr(0, last_slash + 1));
        }
        return stringize("/");
    }

    // ── Helpers ────────────────────────────────────────────────────────────

    bool emitting() const {
        return std::all_of(cond_stack_.begin(), cond_stack_.end(),
                           [](const auto& cs) { return cs.emitting; });
    }

    bool at_end() const { return pos_ >= src_.size(); }

    char peek(size_t offset = 0) const {
        return (pos_ + offset < src_.size()) ? src_[pos_ + offset] : '\0';
    }

    // advance without modifying current_line_
    char eat() {
        return at_end() ? '\0' : src_[pos_++];
    }

    void skip_spaces() { // horizontal whitespace only
        while (!at_end() && (src_[pos_] == ' ' || src_[pos_] == '\t')) pos_++;
    }

    // read to (but not including) the next '\n' or end of string
    std::string read_line_text() {
        std::string r;
        while (!at_end() && src_[pos_] != '\n') {
            if (src_[pos_] == '\\' && pos_ + 1 < src_.size() && src_[pos_+1] == '\n') {
                pos_ += 2;
                current_line_++;
                continue;
            }
            r += src_[pos_++];
        }
        // Strip trailing \r (CRLF line endings)
        if (!r.empty() && r.back() == '\r') r.pop_back();
        return r;
    }

    std::string_view read_ident() {
        size_t start = pos_;
        while (!at_end() && (std::isalnum(static_cast<unsigned char>(src_[pos_])) || src_[pos_] == '_'))
            pos_++;
        return std::string_view(src_).substr(start, pos_ - start);
    }

    void report_error(std::string_view msg) {
        std::string e = std::to_string(current_line_) + ": " + std::string(msg);
        errors_.push_back(e);
        if (emitting()) out_ += "/* PP ERROR: " + e + " */ ";
    }

    // Skip a /* ... */ block comment; called after "/*" has been consumed
    void skip_block_comment() {
        while (!at_end()) {
            char c = eat();
            if (c == '\n') { current_line_++; if (emitting()) out_ += '\n'; }
            if (c == '*' && peek() == '/') { eat(); return; }
        }
        report_error("unterminated block comment");
    }

    // Skip a // line comment; stops before '\n'
    void skip_line_comment() {
        while (!at_end() && src_[pos_] != '\n') pos_++;
    }

    // ── Macro expansion ────────────────────────────────────────────────────

    // Collect function-like macro arguments from 'text' starting at index i.
    // i must point past the opening '('.  Returns args; advances i past ')'.
    static std::vector<std::string> collect_args(std::string_view text, size_t& i) {
        std::vector<std::string> args;
        std::string arg;
        int depth = 0;
        char inq = 0;
        while (i < text.size()) {
            char c = text[i++];
            if (inq) {
                if (c == '\\' && i < text.size()) { arg += c; arg += text[i++]; continue; }
                arg += c;
                if (c == inq) inq = 0;
            } else if (c == '"' || c == '\'') {
                inq = c; arg += c;
            } else if (c == '(') {
                depth++; arg += c;
            } else if (c == ')') {
                if (depth == 0) { args.push_back(std::string(trim(arg))); break; }
                depth--; arg += c;
            } else if (c == ',' && depth == 0) {
                args.push_back(std::string(trim(arg))); arg.clear();
            } else {
                arg += c;
            }
        }
        return args;
    }

    // Substitute 'args' into the macro body, replacing parameter occurrences.
    // Supports stringizing (#) and token pasting (##).
    static std::string substitute(std::string_view body,
                                  const std::vector<std::string>& params,
                                  const std::vector<std::string>& args) {
        std::string temp;
        size_t i = 0;
        while (i < body.size()) {
            // Check for stringizing operator '#'
            if (body[i] == '#') {
                // If it's '##', don't treat it as stringizing
                if (i + 1 < body.size() && body[i + 1] == '#') {
                    temp += "##";
                    i += 2;
                    continue;
                }

                // Skip spaces/tabs after '#'
                size_t j = i + 1;
                while (j < body.size() && (body[j] == ' ' || body[j] == '\t')) {
                    j++;
                }

                // Read potential parameter name
                size_t start = j;
                while (j < body.size() && (std::isalnum(static_cast<unsigned char>(body[j])) || body[j] == '_')) {
                    j++;
                }
                std::string_view ident = body.substr(start, j - start);

                bool replaced = false;
                for (size_t p = 0; p < params.size() && p < args.size(); p++) {
                    if (ident == params[p]) {
                        temp += stringize(args[p]);
                        replaced = true;
                        i = j; // Advance past parameter
                        break;
                    }
                }
                if (replaced) {
                    continue;
                }
            }

            // Handle identifier / potential parameter replacement
            if (std::isalpha(static_cast<unsigned char>(body[i])) || body[i] == '_') {
                size_t start = i;
                while (i < body.size() && (std::isalnum(static_cast<unsigned char>(body[i])) || body[i] == '_')) {
                    i++;
                }
                std::string_view tok = body.substr(start, i - start);
                bool replaced = false;
                for (size_t p = 0; p < params.size() && p < args.size(); p++) {
                    if (tok == params[p]) {
                        temp += args[p];
                        replaced = true;
                        break;
                    }
                }
                if (!replaced) {
                    temp += tok;
                }
                continue;
            }

            // Pass string / char literals verbatim
            if (body[i] == '"' || body[i] == '\'') {
                char q = body[i++];
                temp += q;
                while (i < body.size() && body[i] != q) {
                    if (body[i] == '\\') temp += body[i++];
                    if (i < body.size()) temp += body[i++];
                }
                if (i < body.size()) temp += body[i++];
                continue;
            }

            temp += body[i++];
        }

        // Handle token pasting '##': remove it and any surrounding spaces/tabs,
        // but skip ## that appear inside string or char literals.
        std::string result;
        i = 0;
        while (i < temp.size()) {
            // Pass string/char literals through without scanning for ##
            if (temp[i] == '"' || temp[i] == '\'') {
                char q = temp[i++];
                result += q;
                while (i < temp.size() && temp[i] != q) {
                    if (temp[i] == '\\') result += temp[i++];
                    if (i < temp.size()) result += temp[i++];
                }
                if (i < temp.size()) result += temp[i++];
                continue;
            }

            size_t j = i;
            while (j < temp.size() && (temp[j] == ' ' || temp[j] == '\t')) {
                j++;
            }
            if (j + 1 < temp.size() && temp[j] == '#' && temp[j + 1] == '#') {
                // strip trailing whitespace from result
                while (!result.empty() && (result.back() == ' ' || result.back() == '\t'))
                    result.pop_back();
                j += 2;
                while (j < temp.size() && (temp[j] == ' ' || temp[j] == '\t')) {
                    j++;
                }
                i = j;
                continue;
            }
            result += temp[i++];
        }

        return result;
    }

    // Recursively expand macros in 'text'.
    // 'guard' prevents infinite recursion on the macro currently being expanded.
    std::string expand(std::string_view text,
                       std::vector<std::string_view> guard = {}) const {
        std::string result;
        size_t i = 0;
        while (i < text.size()) {
            // pass string/char literals through unexpanded
            if (text[i] == '"' || text[i] == '\'') {
                char q = text[i++];
                result += q;
                while (i < text.size() && text[i] != q) {
                    if (text[i] == '\\') result += text[i++];
                    if (i < text.size()) result += text[i++];
                }
                if (i < text.size()) result += text[i++];
                continue;
            }
            // identifier?
            if (std::isalpha(static_cast<unsigned char>(text[i])) || text[i] == '_') {
                size_t start = i;
                while (i < text.size() && (std::isalnum(static_cast<unsigned char>(text[i])) || text[i] == '_')) i++;
                std::string_view id = text.substr(start, i - start);

                // Check defined operator
                if (id == "defined") {
                    // find the macro name
                    size_t j = i;
                    while (j < text.size() && (text[j] == ' ' || text[j] == '\t')) j++;
                    bool has_paren = false;
                    if (j < text.size() && text[j] == '(') {
                        has_paren = true;
                        j++;
                        while (j < text.size() && (text[j] == ' ' || text[j] == '\t')) j++;
                    }
                    size_t m_start = j;
                    while (j < text.size() && (std::isalnum(static_cast<unsigned char>(text[j])) || text[j] == '_')) j++;
                    std::string_view macro_name = text.substr(m_start, j - m_start);
                    if (has_paren) {
                        while (j < text.size() && (text[j] == ' ' || text[j] == '\t')) j++;
                        if (j < text.size() && text[j] == ')') j++;
                    }
                    bool is_def = (defines_.count(std::string(macro_name)) > 0 ||
                                   macro_name == "__FILE__" ||
                                   macro_name == "__LINE__" ||
                                   macro_name == "__DIR__");
                    result += is_def ? "1" : "0";
                    i = j;
                    continue;
                }

                // efun_defined(name) — check the compiled-in efun table.
                if (id == "efun_defined") {
                    size_t j = i;
                    while (j < text.size() && (text[j] == ' ' || text[j] == '\t')) j++;
                    if (j < text.size() && text[j] == '(') {
                        j++;
                        while (j < text.size() && (text[j] == ' ' || text[j] == '\t')) j++;
                        size_t name_start = j;
                        while (j < text.size() && (std::isalnum(static_cast<unsigned char>(text[j])) || text[j] == '_')) j++;
                        std::string efun_name(text.substr(name_start, j - name_start));
                        while (j < text.size() && text[j] != ')') j++;
                        if (j < text.size()) j++; // consume ')'
                        result += (lookup_predef(efun_name.c_str()) >= 0) ? '1' : '0';
                        i = j;
                        continue;
                    }
                }

                // Check special macros
                if (id == "__LINE__") {
                    result += std::to_string(current_line_);
                    continue;
                }
                if (id == "__FILE__") {
                    result += stringize(filename_);
                    continue;
                }
                if (id == "__DIR__") {
                    result += get_dir_macro();
                    continue;
                }

                // check recursion guard
                bool guarded = std::any_of(guard.begin(), guard.end(),
                                          [id](std::string_view g) { return g == id; });

                auto it = defines_.find(std::string(id));
                if (!guarded && it != defines_.end()) {
                    const PpMacro& m = it->second;
                    if (!m.is_function_like) {
                        auto g2 = guard; g2.push_back(id);
                        result += expand(m.body, g2);
                    } else {
                        // look for '('
                        size_t j = i;
                        while (j < text.size() && (text[j] == ' ' || text[j] == '\t')) j++;
                        if (j < text.size() && text[j] == '(') {
                            j++; // past '('
                            auto args = collect_args(text, j);
                            i = j;
                            // expand each arg, then substitute
                            std::vector<std::string> expanded_args;
                            auto g2 = guard; g2.push_back(id);
                            for (const auto& a : args) expanded_args.push_back(expand(a, g2));
                            std::string subst = substitute(m.body, m.params, expanded_args);
                            result += expand(subst, g2);
                        } else {
                            result += id; // no '(' — not a call
                        }
                    }
                } else {
                    result += id;
                }
                continue;
            }
            result += text[i++];
        }
        return result;
    }

    // ── Directive handlers ─────────────────────────────────────────────────

    void handle_define(std::string_view rest) {
        size_t i = 0;
        // read macro name
        while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t')) i++;
        size_t ns = i;
        while (i < rest.size() && (std::isalnum(static_cast<unsigned char>(rest[i])) || rest[i] == '_')) i++;
        std::string_view name = rest.substr(ns, i - ns);
        if (name.empty()) { report_error("#define: missing name"); return; }

        auto it = defines_.find(std::string(name));
        if (it != defines_.end() && it->second.is_predefined) {
            report_error("Illegal to redefine a predefined value.");
            return;
        }

        PpMacro m;
        if (i < rest.size() && rest[i] == '(') {
            m.is_function_like = true;
            i++; // skip '('
            while (i < rest.size() && rest[i] != ')') {
                while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t')) i++;
                size_t ps = i;
                while (i < rest.size() && (std::isalnum(static_cast<unsigned char>(rest[i])) || rest[i] == '_')) i++;
                if (i > ps) m.params.emplace_back(rest.substr(ps, i - ps));
                while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t')) i++;
                if (i < rest.size() && rest[i] == ',') i++;
            }
            if (i < rest.size()) i++; // skip ')'
        }
        // body: skip all spaces/tabs
        while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t')) i++;
        std::string body = (i < rest.size()) ? std::string(rest.substr(i)) : "";

        // ## must not appear at the start or end of a macro body
        std::string_view bv(body);
        auto bv_trim = trim(bv);
        if (bv_trim.size() >= 2 && bv_trim.substr(0, 2) == "##") {
            report_error("'##' cannot appear at start of macro body");
            return;
        }
        if (bv_trim.size() >= 2 && bv_trim.substr(bv_trim.size() - 2) == "##") {
            report_error("'##' cannot appear at end of macro body");
            return;
        }

        // Warn on redefinition with a different body
        auto existing = defines_.find(std::string(name));
        if (existing != defines_.end() && !existing->second.is_predefined) {
            if (existing->second.body != body) {
                report_error(std::string("Warning: ") + std::string(name) + " redefined");
            }
        }

        m.body = std::move(body);
        defines_[std::string(name)] = std::move(m);
    }

    long eval_expr(std::string_view expr_in) {
        std::string expr = expand(strip_directive_comments(expr_in));
        std::string trimmed(trim(expr));
        if (trimmed.empty()) {
            report_error("missing expression in #if");
            return 0;
        }
        IfExprParser parser(trimmed);
        long result = parser.parse();
        if (parser.has_error()) {
            report_error(parser.error());
        } else if (!parser.at_end_of_expr()) {
            report_error("Condition too complex");
        }
        return result;
    }

    // ── Main loop ──────────────────────────────────────────────────────────

    std::string run() {
        out_.reserve(src_.size());

        while (!at_end()) {
            // Line continuation outside directives (rare, but valid)
            if (peek() == '\\' && peek(1) == '\n') {
                pos_ += 2; current_line_++;
                continue;
            }

            // Here-doc: @TERM or @@TERM (prevents macro expansion inside raw strings/arrays)
            if (peek() == '@') {
                size_t saved_pos = pos_;
                pos_++;
                bool double_at = false;
                if (peek() == '@') {
                    double_at = true;
                    pos_++;
                }
                skip_spaces();
                if (std::isalpha(static_cast<unsigned char>(peek())) || peek() == '_') {
                    std::string_view term = read_ident();
                    if (!term.empty()) {
                        if (emitting()) {
                            out_ += '@';
                            if (double_at) out_ += '@';
                            out_ += term;
                        }
                        while (!at_end()) {
                            char c = src_[pos_++];
                            if (emitting()) out_ += c;
                            if (c == '\n') {
                                current_line_++;
                                break;
                            }
                        }
                        bool found_term = false;
                        while (!at_end()) {
                            bool matches = true;
                            for (size_t k = 0; k < term.size(); k++) {
                                if (pos_ + k >= src_.size() || src_[pos_ + k] != term[k]) {
                                    matches = false;
                                    break;
                                }
                            }
                            if (matches) {
                                size_t next_idx = pos_ + term.size();
                                char next_c = (next_idx < src_.size()) ? src_[next_idx] : '\0';
                                // Terminator ends on any non-alphanumeric, non-underscore char
                                bool is_word_char = std::isalnum(static_cast<unsigned char>(next_c)) || next_c == '_';
                                if (!is_word_char) {
                                    if (emitting()) out_ += term;
                                    pos_ = next_idx;
                                    found_term = true;
                                    break;
                                }
                            }
                            while (pos_ < src_.size()) {
                                char c = src_[pos_++];
                                if (emitting()) out_ += c;
                                if (c == '\n') {
                                    current_line_++;
                                    break;
                                }
                            }
                        }
                        if (!found_term) {
                            report_error("Unterminated here-doc (EOF before terminator)");
                        }
                        continue;
                    }
                }
                pos_ = saved_pos;
            }

            // Block comment
            if (peek() == '/' && peek(1) == '*') {
                pos_ += 2;
                if (emitting()) out_ += ' ';
                skip_block_comment();
                continue;
            }

            // Line comment
            if (peek() == '/' && peek(1) == '/') {
                pos_ += 2;
                skip_line_comment();
                continue;
            }

            // Token pasting '##'
            if (peek() == '#' && peek(1) == '#') {
                pos_ += 2;
                if (emitting()) {
                    while (!out_.empty() && (out_.back() == ' ' || out_.back() == '\t')) {
                        out_.pop_back();
                    }
                    while (!at_end() && (peek() == ' ' || peek() == '\t')) {
                        pos_++;
                    }
                }
                continue;
            }

            // Preprocessor directive
            if (peek() == '#') {
                bool first_on_line = true;
                for (size_t k = pos_; k > 0; ) {
                    k--;
                    char c = src_[k];
                    if (c == '\n') break;
                    if (c != ' ' && c != '\t') {
                        first_on_line = false;
                        break;
                    }
                }
                if (!first_on_line) {
                    if (emitting()) out_ += src_[pos_];
                    pos_++;
                    continue;
                }
                pos_++;
                skip_spaces();
                std::string_view dir = read_ident();
                skip_spaces();
                std::string rest = read_line_text();  // up to (not incl.) '\n'

                if (dir == "define") {
                    if (emitting()) handle_define(rest);

                } else if (dir == "undef") {
                    if (emitting()) {
                        std::string name(trim(rest));
                        auto it = defines_.find(name);
                        if (it != defines_.end()) {
                            if (it->second.is_predefined) {
                                report_error("Illegal to #undef a predefined value.");
                            } else {
                                defines_.erase(it);
                            }
                        }
                    }

                } else if (dir == "ifdef") {
                    bool def = defines_.count(std::string(trim(rest))) > 0;
                    bool emit = emitting() && def;
                    cond_stack_.push_back({emit, emit});

                } else if (dir == "ifndef") {
                    bool def = defines_.count(std::string(trim(rest))) > 0;
                    bool emit = emitting() && !def;
                    cond_stack_.push_back({emit, emit});

                } else if (dir == "if") {
                    bool cond = emitting() && (eval_expr(rest) != 0);
                    cond_stack_.push_back({cond, cond});

                } else if (dir == "elif") {
                    if (cond_stack_.empty()) { report_error("unexpected #elif"); }
                    else {
                        bool had = cond_stack_.back().had_true;
                        cond_stack_.pop_back();
                        bool outer = emitting();
                        bool now   = outer && !had && (eval_expr(rest) != 0);
                        cond_stack_.push_back({now, had || now});
                    }

                } else if (dir == "else") {
                    if (cond_stack_.empty()) { report_error("unexpected #else"); }
                    else {
                        bool had = cond_stack_.back().had_true;
                        cond_stack_.pop_back();
                        bool now = emitting() && !had;
                        cond_stack_.push_back({now, true});
                    }

                } else if (dir == "endif") {
                    if (cond_stack_.empty()) report_error("unexpected #endif");
                    else cond_stack_.pop_back();

                } else if (dir == "include") {
                    if (emitting()) {
                        if (include_depth_ >= 32) {
                            report_error("#include nested too deeply");
                        } else {
                            std::string name_expr(trim(rest));
                            if (!name_expr.empty() && name_expr[0] != '"' && name_expr[0] != '<') {
                                name_expr = expand(name_expr);
                                name_expr = std::string(trim(name_expr));
                            }
                            if (name_expr.size() >= 2 && (name_expr[0] == '"' || name_expr[0] == '<')) {
                                char delim = name_expr[0] == '"' ? '"' : '>';
                                std::string filename(name_expr.substr(1, name_expr.size() - 2));

                                auto [fd, resolved] = inc_open(filename, delim == '"');
                                if (fd != -1) {
                                    auto include_stream = std::make_unique<FileLexStream>(fd);
                                    // Emit #line for entering the included file
                                    out_ += "\n#line 1 " + stringize(filename) + "\n";

                                    // Recursively preprocess!
                                    Impl sub_impl(std::move(include_stream), filename.c_str(),
                                                  defines_, errors_, include_depth_ + 1);
                                    out_ += sub_impl.run();

                                    // Emit #line for returning to this file
                                    out_ += "\n#line " + std::to_string(current_line_) + " " + stringize(filename_) + "\n";
                                } else {
                                    report_error("Cannot #include " + filename);
                                }
                            } else {
                                report_error("Bad #include directive");
                            }
                        }
                    }

                } else if (dir == "error") {
                    if (emitting()) {
                        report_error("#error " + std::string(trim(rest)));
                    }

                } else if (dir == "warn") {
                    if (emitting()) {
                        report_error("#warn " + std::string(trim(rest)));
                    }

                } else if (dir == "echo") {
                    if (emitting()) {
                        // #echo prints to stderr at compile time
                        fprintf(stderr, "%s\n", std::string(trim(rest)).c_str());
                    }

                } else if (dir == "breakpoint") {
                    // Recognised but ignored in the preprocessor stage

                } else if (dir == "pragma") {
                    if (emitting()) out_ += "#pragma " + rest;

                } else if (dir == "line" || (dir.empty() && !rest.empty() && std::isdigit(static_cast<unsigned char>(rest[0])))) {
                    std::string_view p = rest;
                    size_t i = 0;
                    while (i < p.size() && std::isspace(static_cast<unsigned char>(p[i]))) i++;
                    if (i < p.size() && std::isdigit(static_cast<unsigned char>(p[i]))) {
                        size_t start = i;
                        while (i < p.size() && std::isdigit(static_cast<unsigned char>(p[i]))) i++;
                        long line_num = std::stol(std::string(p.substr(start, i - start)));
                        current_line_ = static_cast<int>(line_num - 1); // offset by 1 because the directive's newline will increment it
                        
                        while (i < p.size() && std::isspace(static_cast<unsigned char>(p[i]))) i++;
                        if (i < p.size() && p[i] == '"') {
                            i++;
                            size_t file_start = i;
                            while (i < p.size() && p[i] != '"') i++;
                            if (i < p.size() && p[i] == '"') {
                                filename_ = normalize_filename(std::string(p.substr(file_start, i - file_start)).c_str());
                            }
                        }
                    }
                    if (emitting()) {
                        out_ += "#line " + rest;
                    }

                } else if (!dir.empty()) {
                    if (emitting()) {
                        report_error("Unknown preprocessor directive: #" + std::string(dir));
                    }
                }

                // consume the '\n'
                if (!at_end() && src_[pos_] == '\n') {
                    if (emitting()) out_ += '\n';
                    pos_++; current_line_++;
                }
                continue;
            }

            // String / char literals: pass through verbatim
            if (peek() == '"' || peek() == '\'') {
                char q = src_[pos_++];
                if (emitting()) out_ += q;
                while (!at_end() && src_[pos_] != q) {
                    char c = src_[pos_++];
                    if (c == '\n') current_line_++;
                    if (c == '\\' && !at_end()) {
                        if (emitting()) out_ += c;
                        c = src_[pos_++];
                    }
                    if (emitting()) out_ += c;
                }
                if (!at_end()) { if (emitting()) out_ += src_[pos_]; pos_++; }
                continue;
            }

            // Newline
            if (peek() == '\n') {
                if (emitting()) out_ += '\n';
                pos_++; current_line_++;
                continue;
            }

            // Identifier: may be a macro
            if (std::isalpha(static_cast<unsigned char>(peek())) || peek() == '_') {
                size_t start = pos_;
                while (!at_end() && (std::isalnum(static_cast<unsigned char>(src_[pos_])) || src_[pos_] == '_'))
                    pos_++;
                std::string_view id = std::string_view(src_).substr(start, pos_ - start);
                if (!emitting()) continue;

                // Check special macros
                if (id == "__LINE__") {
                    out_ += std::to_string(current_line_);
                    continue;
                }
                if (id == "__FILE__") {
                    out_ += stringize(filename_);
                    continue;
                }
                if (id == "__DIR__") {
                    out_ += get_dir_macro();
                    continue;
                }

                auto it = defines_.find(std::string(id));
                if (it == defines_.end()) { out_ += id; continue; }

                const PpMacro& m = it->second;
                if (!m.is_function_like) {
                    out_ += expand(m.body, {id});
                    continue;
                }

                // function-like: collect '(' ... ')'
                size_t saved = pos_;
                while (pos_ < src_.size() && (src_[pos_] == ' ' || src_[pos_] == '\t')) pos_++;
                if (pos_ >= src_.size() || src_[pos_] != '(') {
                    pos_ = saved; out_ += id; continue;
                }
                pos_++; // skip '('
                // collect args from source
                std::vector<std::string> args;
                std::string arg;
                int depth = 0; char inq = 0;
                while (pos_ < src_.size()) {
                    char c = src_[pos_++];
                    if (c == '\n') current_line_++;
                    if (inq) {
                        if (c == '\\' && pos_ < src_.size()) { arg += c; arg += src_[pos_++]; continue; }
                        arg += c; if (c == inq) inq = 0;
                    } else if (c == '"' || c == '\'') { inq = c; arg += c; }
                    else if (c == '(') { depth++; arg += c; }
                    else if (c == ')') {
                        if (depth == 0) { args.push_back(std::string(trim(arg))); break; }
                        depth--; arg += c;
                    } else if (c == ',' && depth == 0) { args.push_back(std::string(trim(arg))); arg.clear(); }
                    else { arg += c; }
                }
                // expand each arg, substitute, re-expand
                std::vector<std::string> eargs;
                for (const auto& a : args) eargs.push_back(expand(a, {id}));
                std::string subst = substitute(m.body, m.params, eargs);
                out_ += expand(subst, {id});
                continue;
            }

            // Everything else: pass through
            char c = src_[pos_++];
            if (emitting()) out_ += c;
        }
        if (!cond_stack_.empty()) {
            report_error("missing #endif");
        }

        return out_;
    }
};

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

LpcPreprocessor::LpcPreprocessor(std::unique_ptr<LexStream> stream,
                                 const char * filename)
    : impl_(std::make_unique<Impl>(std::move(stream), filename)) {}

LpcPreprocessor::LpcPreprocessor(std::string source, const char *filename)
    : impl_(std::make_unique<Impl>(std::move(source), filename)) {}

LpcPreprocessor::~LpcPreprocessor() = default;

std::string LpcPreprocessor::preprocess() {
    return impl_->run();
}

const std::vector<std::string>& LpcPreprocessor::errors() const {
    return impl_->errors_;
}


