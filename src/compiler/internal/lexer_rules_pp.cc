#include "base/std.h"

#include "compiler/internal/lexer_rules_pp.h"

#include <cctype>
#include <cstdlib>

#include "compiler/internal/lex.h"
#include "compiler/internal/lexer_utils.h"

std::string normalize_filename(const char* filename) {
    if (!filename) return "/unknown";
    if (filename[0] == '/') return filename;
    return "/" + std::string(filename);
}

std::string_view trim(std::string_view s) {
    size_t a = s.find_first_not_of(" \t\r");
    if (a == std::string_view::npos) return "";
    size_t b = s.find_last_not_of(" \t\r");
    return s.substr(a, b - a + 1);
}

std::string strip_directive_comments(std::string_view s) {
    std::string r;
    r.reserve(s.size());
    size_t i = 0;
    while (i < s.size()) {
        if (s[i] == '"' || s[i] == '\'' || s[i] == '`') {
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

std::vector<std::string> collect_args(std::string_view text, size_t& i) {
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
        } else if (c == '"' || c == '\'' || c == '`') {
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

std::string substitute(std::string_view body,
                       const std::vector<std::string>& params,
                       const std::vector<std::string>& args) {
    std::string temp;
    size_t i = 0;
    while (i < body.size()) {
        if (body[i] == '#') {
            if (i + 1 < body.size() && body[i + 1] == '#') {
                temp += "##";
                i += 2;
                continue;
            }

            size_t j = i + 1;
            while (j < body.size() && (body[j] == ' ' || body[j] == '\t')) {
                j++;
            }

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
                    i = j;
                    break;
                }
            }
            if (replaced) {
                continue;
            }
        }

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

    std::string result;
    i = 0;
    while (i < temp.size()) {
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

// ---------------------------------------------------------------------------
// #if/#elif expression evaluator
// ---------------------------------------------------------------------------
//
// Plain struct + free functions taking an explicit state pointer, matching
// the rest of lexer_rules*.cc's style (no classes/methods elsewhere in
// this file or in lexer_rules.cc) -- a recursive-descent evaluator built
// the same way lex.l's own escape-decoding helpers are: small functions
// each handling one grammar piece, threading state explicitly instead of
// through `this`.

namespace {

struct IfExprState {
    std::string_view expr;
    size_t pos = 0;
    std::string error;  // first error wins; empty means no error yet
};

bool ifexpr_at_end(const IfExprState *st) { return st->pos >= st->expr.size(); }

char ifexpr_peek(const IfExprState *st) { return ifexpr_at_end(st) ? '\0' : st->expr[st->pos]; }

char ifexpr_advance(IfExprState *st) { return ifexpr_at_end(st) ? '\0' : st->expr[st->pos++]; }

void ifexpr_skip_ws(IfExprState *st) {
    while (!ifexpr_at_end(st) && std::isspace(static_cast<unsigned char>(st->expr[st->pos])))
        st->pos++;
}

void ifexpr_set_error(IfExprState *st, const char *msg) {
    if (st->error.empty()) st->error = msg;
}

// True once only whitespace (or nothing) remains -- used after a top-level
// parse to detect trailing junk the grammar didn't account for.
bool ifexpr_at_end_of_expr(const IfExprState *st) {
    size_t p = st->pos;
    while (p < st->expr.size() && std::isspace(static_cast<unsigned char>(st->expr[p]))) p++;
    return p >= st->expr.size();
}

long ifexpr_top(IfExprState *st);

long ifexpr_atom(IfExprState *st) {
    ifexpr_skip_ws(st);
    if (ifexpr_at_end(st)) return 0;

    char c = ifexpr_peek(st);

    if (c == '(') {
        ifexpr_advance(st);
        long v = ifexpr_top(st);
        ifexpr_skip_ws(st);
        if (ifexpr_peek(st) == ')') {
            ifexpr_advance(st);
        } else {
            ifexpr_set_error(st, "bracket not paired in #if");
        }
        return v;
    }
    if (c == '!') { ifexpr_advance(st); return !ifexpr_atom(st); }
    if (c == '~') { ifexpr_advance(st); return ~ifexpr_atom(st); }
    if (c == '-') { ifexpr_advance(st); return -ifexpr_atom(st); }
    if (c == '+') { ifexpr_advance(st); return ifexpr_atom(st); }

    if (c == '\'') {
        ifexpr_advance(st);
        long value = 0;
        if (!ifexpr_at_end(st) && ifexpr_peek(st) == '\\') {
            ifexpr_advance(st);
            if (!ifexpr_at_end(st)) {
                char esc = ifexpr_advance(st);
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
                        long hv = 0;
                        while (!ifexpr_at_end(st) && std::isxdigit(static_cast<unsigned char>(ifexpr_peek(st)))) {
                            char h = ifexpr_advance(st);
                            hv = (hv * 16) + (std::isdigit(static_cast<unsigned char>(h))
                                ? h - '0'
                                : std::tolower(static_cast<unsigned char>(h)) - 'a' + 10);
                        }
                        value = hv;
                        break;
                    }
                    default:
                        if (std::isdigit(static_cast<unsigned char>(esc))) {
                            long ov = esc - '0';
                            for (int i = 0; i < 2 && !ifexpr_at_end(st) && ifexpr_peek(st) >= '0' && ifexpr_peek(st) <= '7'; i++) {
                                ov = (ov * 8) + (ifexpr_advance(st) - '0');
                            }
                            value = ov;
                        } else {
                            value = static_cast<unsigned char>(esc);
                        }
                }
            }
        } else if (!ifexpr_at_end(st) && ifexpr_peek(st) != '\'') {
            value = static_cast<unsigned char>(ifexpr_advance(st));
        }
        if (!ifexpr_at_end(st) && ifexpr_peek(st) == '\'') ifexpr_advance(st);
        return value;
    }

    if (std::isdigit(static_cast<unsigned char>(c))) {
        size_t start = st->pos;
        if (c == '0' && st->pos + 1 < st->expr.size() &&
            (st->expr[st->pos + 1] == 'x' || st->expr[st->pos + 1] == 'X')) {
            st->pos += 2;
            while (!ifexpr_at_end(st) && std::isxdigit(static_cast<unsigned char>(st->expr[st->pos])))
                st->pos++;
        } else {
            while (!ifexpr_at_end(st) && std::isdigit(static_cast<unsigned char>(st->expr[st->pos])))
                st->pos++;
        }
        while (!ifexpr_at_end(st) && (st->expr[st->pos] == 'L' || st->expr[st->pos] == 'U' ||
                                       st->expr[st->pos] == 'l' || st->expr[st->pos] == 'u'))
            st->pos++;
        std::string literal(st->expr.substr(start, st->pos - start));
        return strtol(literal.c_str(), nullptr, 0);
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        while (!ifexpr_at_end(st) && (std::isalnum(static_cast<unsigned char>(st->expr[st->pos])) || st->expr[st->pos] == '_'))
            st->pos++;
        return 0;
    }

    ifexpr_advance(st);
    return 0;
}

long ifexpr_binop(IfExprState *st, int min_prec) {
    long lhs = ifexpr_atom(st);
    for (;;) {
        ifexpr_skip_ws(st);
        if (ifexpr_at_end(st)) break;

        char c0 = st->expr[st->pos];
        char c1 = (st->pos + 1 < st->expr.size()) ? st->expr[st->pos + 1] : '\0';

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

        st->pos += op_len;
        long rhs = ifexpr_binop(st, prec + 1);

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
                if (rhs == 0) { ifexpr_set_error(st, "division by 0 in #if"); lhs = 0; }
                else { lhs = lhs / rhs; }
                break;
            case '%':
                if (rhs == 0) { ifexpr_set_error(st, "modulo by 0 in #if"); lhs = 0; }
                else { lhs = lhs % rhs; }
                break;
            default: break;
        }
    }
    return lhs;
}

long ifexpr_top(IfExprState *st) {
    long cond = ifexpr_binop(st, 0);
    ifexpr_skip_ws(st);
    if (!ifexpr_at_end(st) && ifexpr_peek(st) == '?') {
        ifexpr_advance(st);
        ifexpr_skip_ws(st);
        long true_val = ifexpr_top(st);
        ifexpr_skip_ws(st);
        if (!ifexpr_at_end(st) && ifexpr_peek(st) == ':') {
            ifexpr_advance(st);
        } else {
            ifexpr_set_error(st, "'?' without ':' in #if");
        }
        ifexpr_skip_ws(st);
        long false_val = ifexpr_top(st);
        return cond ? true_val : false_val;
    }
    return cond;
}

}  // namespace

long lpc_lex_eval_if_expr(std::string_view expr) {
    IfExprState st;
    st.expr = expr;
    long result = ifexpr_top(&st);
    if (!st.error.empty()) {
        lexerror(st.error.c_str());
        return 0;
    }
    if (!ifexpr_at_end_of_expr(&st)) {
        lexerror("Condition too complex");
        return 0;
    }
    return result;
}

void LexerSession::add_builtin_macros() {
    const auto& predefs = get_predefines();
    for (const auto& pair : predefs) {
        PpMacro m;
        m.is_function_like = pair.second.is_function_like;
        m.is_predefined = true;
        m.body = pair.second.body;
        macros[pair.first] = std::move(m);
    }
    macros["__FILE__"] = PpMacro{false, true, {}, ""};
    macros["__LINE__"] = PpMacro{false, true, {}, ""};
    macros["__DIR__"] = PpMacro{false, true, {}, ""};
}

bool lpc_lex_emitting() {
    if (!current_session) return true;
    for (const auto& cs : current_session->conds) {
        if (!cs.emitting) return false;
    }
    return true;
}

// current_line/total_lines += for each '\n' embedded in the matched text
// (backslash-continuation line breaks inside a directive). Called exactly
// once per captured line, by lpc_lex_on_directive().
static void count_directive_newlines(const char* text, int len) {
    for (int i = 0; i < len; i++) {
        if (text[i] == '\n') {
            current_line++;
            total_lines++;
        }
    }
}

bool lpc_lex_builtin_macro(std::string_view name, std::string* out) {
    if (name == "__LINE__") {
        *out = std::to_string(current_line);
        return true;
    }
    if (name == "__FILE__") {
        *out = stringize(normalize_filename(current_file));
        return true;
    }
    if (name == "__DIR__") {
        std::string filename = normalize_filename(current_file);
        size_t last_slash = filename.find_last_of('/');
        // normalize_filename() guarantees a leading '/', so there is always
        // at least one slash to keep.
        *out = stringize(filename.substr(0, last_slash + 1));
        return true;
    }
    return false;
}

std::string lpc_lex_expand_string(std::string_view text, std::vector<std::string> guard, bool in_if_expr,
                                  std::unordered_map<std::string, int>* guard_hits) {
    if (!current_session) return std::string(text);
    std::string result;
    size_t i = 0;
    while (i < text.size()) {
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
        if (std::isalpha(static_cast<unsigned char>(text[i])) || text[i] == '_') {
            size_t start = i;
            while (i < text.size() && (std::isalnum(static_cast<unsigned char>(text[i])) || text[i] == '_')) i++;
            std::string_view id = text.substr(start, i - start);

            if (in_if_expr && id == "defined") {
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
                bool is_def = (current_session->macros.count(std::string(macro_name)) > 0 ||
                               macro_name == "__FILE__" ||
                               macro_name == "__LINE__" ||
                               macro_name == "__DIR__");
                result += is_def ? "1" : "0";
                i = j;
                continue;
            }

            if (in_if_expr && id == "efun_defined") {
                size_t j = i;
                while (j < text.size() && (text[j] == ' ' || text[j] == '\t')) j++;
                if (j < text.size() && text[j] == '(') {
                    j++;
                    while (j < text.size() && (text[j] == ' ' || text[j] == '\t')) j++;
                    size_t name_start = j;
                    while (j < text.size() && (std::isalnum(static_cast<unsigned char>(text[j])) || text[j] == '_')) j++;
                    std::string efun_name(text.substr(name_start, j - name_start));
                    while (j < text.size() && text[j] != ')') j++;
                    if (j < text.size()) j++;
                    result += (lookup_predef(efun_name.c_str()) >= 0) ? "1" : "0";
                    i = j;
                    continue;
                }
            }

            {
                std::string builtin;
                if (lpc_lex_builtin_macro(id, &builtin)) {
                    result += builtin;
                    continue;
                }
            }

            bool guarded = false;
            for (const auto& g : guard) {
                if (g == id) { guarded = true; break; }
            }

            auto it = current_session->macros.find(std::string(id));
            if (!guarded && it != current_session->macros.end()) {
                const PpMacro& m = it->second;
                if (!m.is_function_like) {
                    auto g2 = guard; g2.push_back(std::string(id));
                    result += lpc_lex_expand_string(m.body, g2, in_if_expr, guard_hits);
                } else {
                    size_t j = i;
                    while (j < text.size() && (text[j] == ' ' || text[j] == '\t')) j++;
                    if (j < text.size() && text[j] == '(') {
                        j++;
                        auto args = collect_args(text, j);
                        i = j;
                        std::vector<std::string> expanded_args;
                        auto g2 = guard; g2.push_back(std::string(id));
                        // Argument pre-expansion deliberately passes no
                        // guard_hits: its output goes through substitute()
                        // and the substituted body is re-walked just below
                        // -- only THAT walk's emissions reach `result`, so
                        // counting here too would double-count any guarded
                        // literal that survives substitution.
                        for (const auto& a : args) expanded_args.push_back(lpc_lex_expand_string(a, guard, in_if_expr));
                        std::string subst = substitute(m.body, m.params, expanded_args);
                        result += lpc_lex_expand_string(subst, g2, in_if_expr, guard_hits);
                    } else {
                        // Function-like macro name with no argument list in
                        // this text: left literal and NOT counted -- if its
                        // '(' turns out to follow in the input stream after
                        // this text is spliced, the rescan may legitimately
                        // expand it there (C behavior).
                        result += id;
                    }
                }
            } else {
                if (guarded && guard_hits) {
                    ++(*guard_hits)[std::string(id)];
                }
                result += id;
            }
            continue;
        }
        result += text[i++];
    }
    return result;
}

static std::string fold_backslash_newlines(std::string_view text) {
    std::string result;
    result.reserve(text.size());
    for (size_t i = 0; i < text.size(); ) {
        if (text[i] == '\\') {
            size_t j = i + 1;
            if (j < text.size() && text[j] == '\r') j++;
            if (j < text.size() && text[j] == '\n') {
                i = j + 1;
                continue;
            }
        }
        result.push_back(text[i]);
        i++;
    }
    return result;
}

// Applies one already-parsed directive: `dir` is the directive keyword,
// `rest` its payload (both views into the caller's folded line). This is
// the single implementation behind lpc_lex_on_directive() -- both scan
// modes funnel through it, so a skip-mode-ending #elif is evaluated by
// exactly the same code as an emit-mode one.
static void dispatch_directive(std::string_view dir, std::string_view rest) {
    if (dir == "define") {
        if (lpc_lex_emitting()) {
            size_t idx = 0;
            while (idx < rest.size() && (rest[idx] == ' ' || rest[idx] == '\t')) idx++;
            size_t ns = idx;
            while (idx < rest.size() && (std::isalnum(static_cast<unsigned char>(rest[idx])) || rest[idx] == '_')) idx++;
            std::string_view name = rest.substr(ns, idx - ns);
            if (name.empty()) { lexerror("#define: missing name"); return; }

            auto existing = current_session->macros.find(std::string(name));
            if (existing != current_session->macros.end() && existing->second.is_predefined) {
                lexerror("Illegal to redefine a predefined value.");
                return;
            }

            PpMacro m;
            if (idx < rest.size() && rest[idx] == '(') {
                m.is_function_like = true;
                idx++; // skip '('
                while (idx < rest.size() && rest[idx] != ')') {
                    while (idx < rest.size() && (rest[idx] == ' ' || rest[idx] == '\t')) idx++;
                    size_t ps = idx;
                    while (idx < rest.size() && (std::isalnum(static_cast<unsigned char>(rest[idx])) || rest[idx] == '_')) idx++;
                    if (idx > ps) m.params.emplace_back(rest.substr(ps, idx - ps));
                    while (idx < rest.size() && (rest[idx] == ' ' || rest[idx] == '\t')) idx++;
                    if (idx < rest.size() && rest[idx] == ',') idx++;
                }
                if (idx < rest.size()) idx++; // skip ')'
            }
            while (idx < rest.size() && (rest[idx] == ' ' || rest[idx] == '\t')) idx++;
            std::string body = (idx < rest.size()) ? std::string(rest.substr(idx)) : "";

            std::string_view bv(body);
            auto bv_trim = trim(bv);
            if (bv_trim.size() >= 2 && bv_trim.substr(0, 2) == "##") {
                lexerror("'##' cannot appear at start of macro body");
                return;
            }
            if (bv_trim.size() >= 2 && bv_trim.substr(bv_trim.size() - 2) == "##") {
                lexerror("'##' cannot appear at end of macro body");
                return;
            }

            if (existing != current_session->macros.end() && !existing->second.is_predefined) {
                if (existing->second.body != body) {
                    lexerror((std::string("Warning: ") + std::string(name) + " redefined").c_str());
                }
            }

            m.body = std::move(body);
            current_session->macros[std::string(name)] = std::move(m);
        }
    } else if (dir == "undef") {
        if (lpc_lex_emitting()) {
            std::string name(trim(rest));
            auto it = current_session->macros.find(name);
            if (it != current_session->macros.end()) {
                if (it->second.is_predefined) {
                    lexerror("Illegal to #undef a predefined value.");
                } else {
                    current_session->macros.erase(name);
                }
            }
        }
    } else if (dir == "ifdef") {
        bool def = current_session->macros.count(std::string(trim(rest))) > 0;
        bool emit = lpc_lex_emitting() && def;
        current_session->conds.push_back({emit, emit});
    } else if (dir == "ifndef") {
        bool def = current_session->macros.count(std::string(trim(rest))) > 0;
        bool emit = lpc_lex_emitting() && !def;
        current_session->conds.push_back({emit, emit});
    } else if (dir == "if") {
        bool cond = false;
        if (lpc_lex_emitting()) {
            std::string expr = lpc_lex_expand_string(strip_directive_comments(rest), {}, /*in_if_expr=*/true);
            std::string trimmed(trim(expr));
            if (trimmed.empty()) {
                lexerror("missing expression in #if");
            } else {
                cond = (lpc_lex_eval_if_expr(trimmed) != 0);
            }
        }
        current_session->conds.push_back({cond, cond});
    } else if (dir == "elif") {
        if (current_session->conds.empty()) {
            lexerror("unexpected #elif");
        } else {
            bool had = current_session->conds.back().had_true;
            current_session->conds.pop_back();
            bool outer = lpc_lex_emitting();
            bool cond = false;
            if (outer && !had) {
                std::string expr = lpc_lex_expand_string(strip_directive_comments(rest), {}, /*in_if_expr=*/true);
                std::string trimmed(trim(expr));
                if (trimmed.empty()) {
                    lexerror("missing expression in #elif");
                } else {
                    cond = (lpc_lex_eval_if_expr(trimmed) != 0);
                }
            }
            bool now = outer && !had && cond;
            current_session->conds.push_back({now, had || now});
        }
    } else if (dir == "else") {
        if (current_session->conds.empty()) {
            lexerror("unexpected #else");
        } else {
            bool had = current_session->conds.back().had_true;
            current_session->conds.pop_back();
            bool now = lpc_lex_emitting() && !had;
            current_session->conds.push_back({now, true});
        }
    } else if (dir == "endif") {
        if (current_session->conds.empty()) {
            lexerror("unexpected #endif");
        } else {
            current_session->conds.pop_back();
        }
    } else if (dir == "include") {
        if (lpc_lex_emitting()) {
            lpc_lex_handle_include(rest);
        }
    } else if (dir == "error") {
        if (lpc_lex_emitting()) {
            lexerror(("#error " + std::string(trim(rest))).c_str());
        }
    } else if (dir == "warn") {
        if (lpc_lex_emitting()) {
            lexerror(("#warn " + std::string(trim(rest))).c_str());
        }
    } else if (dir == "echo") {
        if (lpc_lex_emitting()) {
            fprintf(stderr, "%s\n", std::string(trim(rest)).c_str());
        }
    } else if (dir == "pragma") {
        if (lpc_lex_emitting()) {
            std::string rest_str(trim(rest));
            handle_pragma(const_cast<char*>(rest_str.c_str()));
        }
    } else if (dir == "line" || (dir.empty() && !rest.empty() && std::isdigit(static_cast<unsigned char>(rest[0])))) {
        std::string_view p = rest;
        size_t idx = 0;
        while (idx < p.size() && std::isspace(static_cast<unsigned char>(p[idx]))) idx++;
        if (idx < p.size() && std::isdigit(static_cast<unsigned char>(p[idx]))) {
            size_t start = idx;
            while (idx < p.size() && std::isdigit(static_cast<unsigned char>(p[idx]))) idx++;
            long line_num = strtol(std::string(p.substr(start, idx - start)).c_str(), nullptr, 10);
            // No -1 here: the lex.l directive rule consumed and counted the
            // directive's terminating newline BEFORE dispatching, so the
            // next line scanned is exactly the one `#line N` names.
            current_line = static_cast<int>(line_num);

            while (idx < p.size() && std::isspace(static_cast<unsigned char>(p[idx]))) idx++;
            if (idx < p.size() && p[idx] == '"') {
                idx++;
                size_t file_start = idx;
                while (idx < p.size() && p[idx] != '"') idx++;
                if (idx < p.size() && p[idx] == '"') {
                    std::string new_file = normalize_filename(std::string(p.substr(file_start, idx - file_start)).c_str());
                    free_string(current_file);
                    current_file = make_shared_string(new_file.c_str());
                }
            }
        }
    } else if (dir == "breakpoint") {
        // Ignored
    } else if (!dir.empty()) {
        if (lpc_lex_emitting()) {
            lexerror(("Unknown preprocessor directive: #" + std::string(dir)).c_str());
        }
    }
}

LpcDirectiveAction lpc_lex_on_directive(const char* text, int len, void* yyscanner,
                                        bool in_skip_mode) {
    // Embedded continuation newlines are physical lines regardless of scan
    // mode or whether the directive dispatches -- counted exactly once,
    // here (the terminating newline is the lex.l rule's job, not ours).
    count_directive_newlines(text, len);

    if (!current_session) return LpcDirectiveAction::kNone;

    // Fold + parse the captured line exactly once: both the skip-mode
    // classification and the full dispatch below read the same name/rest.
    std::string folded = fold_backslash_newlines(std::string_view(text, len));
    std::string_view sv(folded);
    size_t i = 0;
    while (i < sv.size() && (sv[i] == ' ' || sv[i] == '\t')) i++;
    if (i < sv.size() && sv[i] == '#') i++;
    while (i < sv.size() && (sv[i] == ' ' || sv[i] == '\t')) i++;

    size_t name_start = i;
    while (i < sv.size() && (std::isalnum(static_cast<unsigned char>(sv[i])) || sv[i] == '_')) i++;
    std::string_view dir = sv.substr(name_start, i - name_start);

    while (i < sv.size() && (sv[i] == ' ' || sv[i] == '\t')) i++;
    std::string_view rest = sv.substr(i);
    while (!rest.empty() && (rest.back() == '\r' || rest.back() == '\n')) {
        rest.remove_suffix(1);
    }

    auto* ctx = reinterpret_cast<compiler_context_t*>(yyget_extra(yyscanner));

    if (!in_skip_mode) {
        dispatch_directive(dir, rest);
        if (!lpc_lex_emitting()) {
            ctx->skip_depth = 0;
            return LpcDirectiveAction::kEnterSkip;
        }
        return LpcDirectiveAction::kNone;
    }

    // Dead branch: track nesting by name only; dispatch nothing except a
    // directive that can end this branch at depth 0. Everything else in a
    // false branch -- #include, #define, unknown directives -- is skipped
    // without validation, matching the old preprocessor's dead-branch
    // behavior.
    if (dir == "if" || dir == "ifdef" || dir == "ifndef") {
        ctx->skip_depth++;
        return LpcDirectiveAction::kNone;
    }
    if (ctx->skip_depth > 0) {
        if (dir == "endif") {
            ctx->skip_depth--;
        }
        return LpcDirectiveAction::kNone;
    }
    if (dir == "endif" || dir == "elif" || dir == "else") {
        dispatch_directive(dir, rest);
        return lpc_lex_emitting() ? LpcDirectiveAction::kExitSkip : LpcDirectiveAction::kNone;
    }
    return LpcDirectiveAction::kNone;
}

