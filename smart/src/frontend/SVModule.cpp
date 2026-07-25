#include "SVModule.h"

#include <cctype>
#include <cmath>
#include <regex>
#include <sstream>
#include <stdexcept>

// Implementation note: std::regex (libstdc++) overflows the stack on large
// inputs — ISCAS netlists like c2670/s38584 have hundreds of ports and a
// single `(\w+(,\w+)*)`-style pattern over them SIGSEGVs. All scanning over
// whole files/modules is therefore hand-rolled; std::regex only touches
// short strings (range expressions, parameter substitution).

namespace smart {
namespace frontend {

namespace {

const std::vector<std::string> kClockNames = {"clk", "clock", "clk_i", "i_clk"};
const std::vector<std::string> kResetNames = {"rst", "reset", "rst_i", "i_rst",
                                              "rst_n", "rst_ni", "reset_n",
                                              "resetn"};
const std::vector<std::string> kDirections = {"input", "output", "inout"};
const std::vector<std::string> kTypeWords = {"wire", "reg", "logic", "bit", "var"};

bool isWordChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

bool isSpace(char c) { return std::isspace(static_cast<unsigned char>(c)); }

// True when `word` occurs at `pos` with \b boundaries on both sides.
bool wordAt(const std::string& t, size_t pos, const std::string& word) {
    if (t.compare(pos, word.size(), word) != 0) return false;
    if (pos > 0 && isWordChar(t[pos - 1])) return false;
    size_t end = pos + word.size();
    if (end < t.size() && isWordChar(t[end])) return false;
    return true;
}

size_t findWord(const std::string& t, const std::string& word, size_t from) {
    size_t pos = from;
    while ((pos = t.find(word, pos)) != std::string::npos) {
        if (wordAt(t, pos, word)) return pos;
        pos++;
    }
    return std::string::npos;
}

size_t skipSpaces(const std::string& t, size_t pos) {
    while (pos < t.size() && isSpace(t[pos])) pos++;
    return pos;
}

size_t skipWordChars(const std::string& t, size_t pos) {
    while (pos < t.size() && isWordChar(t[pos])) pos++;
    return pos;
}

// gen_bench._strip_comments: line comments and single-line block comments
// become one space; multi-line block comments become their newlines. No
// string-literal awareness, same as the Python.
std::string stripComments(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    size_t i = 0;
    const size_t n = text.size();
    while (i < n) {
        if (text[i] == '/' && i + 1 < n && text[i + 1] == '/') {
            while (i < n && text[i] != '\n') i++;
            out += ' ';
        } else if (text[i] == '/' && i + 1 < n && text[i + 1] == '*') {
            size_t end = text.find("*/", i + 2);
            if (end == std::string::npos) {  // unterminated: Python regex
                out += text.substr(i);       // leaves it untouched
                break;
            }
            size_t stop = end + 2;
            size_t newlines = 0;
            for (size_t k = i; k < stop; k++)
                if (text[k] == '\n') newlines++;
            if (newlines)
                out.append(newlines, '\n');
            else
                out += ' ';
            i = stop;
        } else {
            out += text[i];
            i++;
        }
    }
    return out;
}

std::string escapeRegex(const std::string& s) {
    static const std::string special = R"(\^$.|?*+()[]{})";
    std::string out;
    for (char c : s) {
        if (special.find(c) != std::string::npos) out += '\\';
        out += c;
    }
    return out;
}

// Find `\bmodule\s+<top>\b`; returns position of "module" or npos.
size_t findModuleStart(const std::string& text, const std::string& top,
                       size_t from = 0) {
    size_t pos = from;
    while ((pos = findWord(text, "module", pos)) != std::string::npos) {
        size_t j = skipSpaces(text, pos + 6);
        if (j > pos + 6 && wordAt(text, j, top)) return pos;
        pos++;
    }
    return std::string::npos;
}

// gen_bench._find_module: `\bmodule\s+<top>\b[\s\S]*?\bendmodule\b` (first).
std::string findModule(const std::string& text, const std::string& top) {
    size_t start = 0;
    while ((start = findModuleStart(text, top, start)) != std::string::npos) {
        size_t end = findWord(text, "endmodule", start);
        if (end != std::string::npos)
            return text.substr(start, end + 9 - start);
        start++;
    }
    throw std::runtime_error("module '" + top + "' not found");
}

std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n\f\v");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n\f\v");
    return s.substr(a, b - a + 1);
}

// Tiny arithmetic evaluator over doubles (Python eval uses true division;
// int() truncates toward zero at the end).
struct Arith {
    const std::string& s;
    size_t i = 0;
    bool ok = true;
    explicit Arith(const std::string& str) : s(str) {}
    void skip() { while (i < s.size() && isSpace(s[i])) i++; }
    double factor() {
        skip();
        if (!ok) return 0;
        if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
            char op = s[i++];
            double v = factor();
            return op == '-' ? -v : v;
        }
        if (i < s.size() && s[i] == '(') {
            i++;
            double v = expr();
            skip();
            if (i < s.size() && s[i] == ')') i++;
            else ok = false;
            return v;
        }
        size_t start = i;
        while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) i++;
        if (i == start) { ok = false; return 0; }
        try {
            return std::stod(s.substr(start, i - start));
        } catch (...) {
            ok = false;
            return 0;
        }
    }
    double term() {
        double v = factor();
        while (ok) {
            skip();
            if (i < s.size() && (s[i] == '*' || s[i] == '/' || s[i] == '%')) {
                char op = s[i++];
                double r = factor();
                if ((op == '/' || op == '%') && r == 0) { ok = false; return 0; }
                v = op == '*' ? v * r : op == '/' ? v / r : std::fmod(v, r);
            } else {
                break;
            }
        }
        return v;
    }
    double expr() {
        double v = term();
        while (ok) {
            skip();
            if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
                char op = s[i++];
                double r = term();
                v = op == '+' ? v + r : v - r;
            } else {
                break;
            }
        }
        return v;
    }
};

// gen_bench._safe_eval. Returns nullopt on anything unresolvable.
std::optional<long long> safeEval(std::string expr,
                                  const std::map<std::string, long long>& params) {
    expr = trim(expr);
    // Verilog sized literals like 8'd255 / 4'b1010 / 16'hff (lowercase base
    // letter only, same as the Python regex).
    static const std::regex sized(R"(\d+'([bodh])([0-9a-fA-F_xz?]+))");
    std::smatch m;
    if (std::regex_match(expr, m, sized)) {
        int base = m.str(1) == "b" ? 2 : m.str(1) == "o" ? 8
                 : m.str(1) == "d" ? 10 : 16;
        long long value = 0;
        for (char c : m.str(2)) {
            if (c == '_') continue;
            int digit;
            if (c >= '0' && c <= '9') digit = c - '0';
            else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
            else return std::nullopt;  // x/z/? -> int() raises in Python
            if (digit >= base) return std::nullopt;
            value = value * base + digit;
        }
        return value;
    }
    for (const auto& [name, value] : params) {
        std::regex ref("\\b" + escapeRegex(name) + "\\b");
        expr = std::regex_replace(expr, ref, std::to_string(value));
    }
    if (expr.empty()) return std::nullopt;
    for (char c : expr) {
        if (!(std::isdigit(static_cast<unsigned char>(c)) || isSpace(c) ||
              c == '(' || c == ')' || c == '+' || c == '-' || c == '*' ||
              c == '/' || c == '%'))
            return std::nullopt;
    }
    Arith a(expr);
    double v = a.expr();
    a.skip();
    if (!a.ok || a.i != expr.size() || !std::isfinite(v)) return std::nullopt;
    return static_cast<long long>(std::trunc(v));
}

// gen_bench._range_width: "" -> 1; unresolvable -> -1 (Python None).
int rangeWidth(const std::string& rangeText,
               const std::map<std::string, long long>& params) {
    if (trim(rangeText).empty()) return 1;
    // fullmatch \s*\[(.+):(.+)\]\s* with greedy first group (split at the
    // LAST colon); '.' must not match newline, like Python without DOTALL
    size_t open = rangeText.find('[');
    size_t close = rangeText.rfind(']');
    if (open == std::string::npos || close == std::string::npos || close < open)
        return -1;
    if (trim(rangeText.substr(0, open)) != "" ||
        trim(rangeText.substr(close + 1)) != "")
        return -1;
    std::string inner = rangeText.substr(open + 1, close - open - 1);
    if (inner.find('\n') != std::string::npos) return -1;
    size_t colon = inner.rfind(':');
    if (colon == std::string::npos || colon == 0 || colon == inner.size() - 1)
        return -1;
    auto msb = safeEval(inner.substr(0, colon), params);
    auto lsb = safeEval(inner.substr(colon + 1), params);
    if (!msb || !lsb) return -1;
    return static_cast<int>(std::llabs(*msb - *lsb)) + 1;
}

// gen_bench._parse_parameters:
//   \b(?:parameter|localparam)\b[^=;,)]*?(\w+)\s*=\s*([^,;)\n]+)
std::map<std::string, long long> parseParameters(const std::string& text) {
    std::map<std::string, long long> params;
    size_t pos = 0;
    const size_t n = text.size();
    while (pos < n) {
        size_t p1 = findWord(text, "parameter", pos);
        size_t p2 = findWord(text, "localparam", pos);
        size_t kw = std::min(p1, p2);
        if (kw == std::string::npos) break;
        size_t kwEnd = kw + (kw == p1 ? 9 : 10);

        // first '=' reachable without crossing ; , )
        size_t eq = std::string::npos;
        for (size_t k = kwEnd; k < n; k++) {
            char c = text[k];
            if (c == '=') { eq = k; break; }
            if (c == ';' || c == ',' || c == ')') break;
        }
        if (eq == std::string::npos) { pos = kwEnd; continue; }

        // identifier immediately before '=' (whitespace allowed between)
        size_t idEnd = eq;
        while (idEnd > kwEnd && isSpace(text[idEnd - 1])) idEnd--;
        size_t idStart = idEnd;
        while (idStart > kwEnd && isWordChar(text[idStart - 1])) idStart--;
        if (idStart == idEnd) { pos = kwEnd; continue; }
        std::string name = text.substr(idStart, idEnd - idStart);

        // value: [^,;)\n]+ after '=' (and optional spaces)
        size_t vStart = eq + 1;
        size_t vEnd = vStart;
        while (vEnd < n && text[vEnd] != ',' && text[vEnd] != ';' &&
               text[vEnd] != ')' && text[vEnd] != '\n')
            vEnd++;
        if (vEnd == vStart) { pos = eq + 1; continue; }
        auto value = safeEval(trim(text.substr(vStart, vEnd - vStart)), params);
        if (value) params[name] = *value;
        pos = vEnd;
    }
    return params;
}

// gen_bench._header_and_body.
std::pair<std::string, std::string> headerAndBody(const std::string& moduleText) {
    // re.match(r"\s*module\s+\w+\s*", ...)
    size_t pos = skipSpaces(moduleText, 0);
    pos += 6;  // "module" (findModule guarantees the text starts with it)
    pos = skipSpaces(moduleText, pos);
    pos = skipWordChars(moduleText, pos);
    pos = skipSpaces(moduleText, pos);

    if (pos < moduleText.size() && moduleText[pos] == '#') {
        pos = moduleText.find('(', pos);
        if (pos == std::string::npos)
            throw std::runtime_error("malformed parameter list");
        int depth = 1;
        pos++;
        while (depth) {
            if (pos >= moduleText.size())
                throw std::runtime_error("malformed parameter list");
            if (moduleText[pos] == '(') depth++;
            else if (moduleText[pos] == ')') depth--;
            pos++;
        }
    }
    size_t openParen = moduleText.find('(', pos);
    if (openParen == std::string::npos) return {"", moduleText};
    int depth = 1;
    size_t i = openParen + 1;
    while (depth) {
        if (i >= moduleText.size())
            throw std::runtime_error("malformed port list");
        if (moduleText[i] == '(') depth++;
        else if (moduleText[i] == ')') depth--;
        i++;
    }
    return {moduleText.substr(openParen + 1, i - 1 - (openParen + 1)),
            moduleText.substr(i)};
}

// gen_bench.DIR_RE + _parse_port_decls, hand-rolled.
std::vector<Port> parsePortDecls(const std::string& text,
                                 const std::map<std::string, long long>& params) {
    static const std::vector<std::string> keywords = {
        "input", "output", "inout", "wire", "reg", "logic", "bit",
        "var", "signed", "unsigned"};
    std::vector<Port> ports;
    size_t pos = 0;
    const size_t n = text.size();
    while (pos < n) {
        // next direction keyword
        size_t kw = std::string::npos;
        std::string direction;
        for (const auto& d : kDirections) {
            size_t hit = findWord(text, d, pos);
            if (hit < kw) { kw = hit; direction = d; }
        }
        if (kw == std::string::npos) break;
        size_t p = kw + direction.size();

        // (?:\s+(?:wire|reg|logic|bit|var))*
        while (true) {
            size_t save = p;
            size_t q = skipSpaces(text, p);
            if (q == p) break;  // \s+ required
            bool consumed = false;
            for (const auto& tw : kTypeWords) {
                if (wordAt(text, q, tw)) {
                    p = q + tw.size();
                    consumed = true;
                    break;
                }
            }
            if (!consumed) { p = save; break; }
        }
        // (?:\s+signed|\s+unsigned)?
        {
            size_t q = skipSpaces(text, p);
            if (q > p) {
                if (wordAt(text, q, "signed")) p = q + 6;
                else if (wordAt(text, q, "unsigned")) p = q + 8;
            }
        }
        // \s*((?:\[[^\]]+\]\s*)*)
        p = skipSpaces(text, p);
        std::string firstRange;
        while (p < n && text[p] == '[') {
            size_t close = text.find(']', p + 1);
            if (close == std::string::npos || close == p + 1) break;  // [^\]]+
            if (firstRange.empty())
                firstRange = text.substr(p, close + 1 - p);
            p = skipSpaces(text, close + 1);
        }
        // \s*(\w+(?:\s*,\s*(?!input\b|output\b|inout\b)\w+)*)
        p = skipSpaces(text, p);
        std::vector<std::string> names;
        size_t nameEnd = skipWordChars(text, p);
        if (nameEnd == p) { pos = kw + 1; continue; }  // no match here
        names.push_back(text.substr(p, nameEnd - p));
        p = nameEnd;
        while (true) {
            size_t save = p;
            size_t q = skipSpaces(text, p);
            if (q >= n || text[q] != ',') break;
            q = skipSpaces(text, q + 1);
            bool isDir = false;
            for (const auto& d : kDirections)
                if (wordAt(text, q, d)) { isDir = true; break; }
            if (isDir) { p = save; break; }  // lookahead (?!input|output|inout)
            size_t e = skipWordChars(text, q);
            if (e == q) { p = save; break; }
            names.push_back(text.substr(q, e - q));
            p = e;
        }

        int width = rangeWidth(firstRange, params);
        for (const auto& name : names) {
            bool isKeyword = false;
            for (const auto& k : keywords)
                if (name == k) { isKeyword = true; break; }
            if (isKeyword) break;  // next declaration's keyword ends the list
            ports.push_back({direction, width, name});
        }
        pos = p;
    }
    return ports;
}

// gen_bench.FREE_REG_RE, hand-rolled:
//   \(\*\s*(anyseq|anyconst)\s*\*\)\s*(?:reg|logic|wire|bit)\s*
//   (?:signed\s+|unsigned\s+)?((?:\[[^\]]+\]\s*)?)(\w+)
std::vector<FreeReg> parseFreeRegs(const std::string& text,
                                   const std::map<std::string, long long>& params) {
    static const std::vector<std::string> kRegTypes = {"reg", "logic", "wire",
                                                       "bit"};
    std::vector<FreeReg> regs;
    size_t pos = 0;
    const size_t n = text.size();
    while ((pos = text.find("(*", pos)) != std::string::npos) {
        size_t p = skipSpaces(text, pos + 2);
        std::string kind;
        if (wordAt(text, p, "anyseq")) kind = "anyseq";
        else if (wordAt(text, p, "anyconst")) kind = "anyconst";
        if (kind.empty()) { pos += 2; continue; }
        p = skipSpaces(text, p + kind.size());
        if (text.compare(p, 2, "*)") != 0) { pos += 2; continue; }
        p = skipSpaces(text, p + 2);
        bool typed = false;
        for (const auto& tw : kRegTypes) {
            if (wordAt(text, p, tw)) {
                p += tw.size();
                typed = true;
                break;
            }
        }
        if (!typed) { pos += 2; continue; }
        p = skipSpaces(text, p);
        // (?:signed\s+|unsigned\s+)? — trailing whitespace REQUIRED
        for (const std::string su : {"signed", "unsigned"}) {
            if (text.compare(p, su.size(), su) == 0 && p + su.size() < n &&
                isSpace(text[p + su.size()])) {
                p = skipSpaces(text, p + su.size());
                break;
            }
        }
        std::string range;
        if (p < n && text[p] == '[') {
            size_t close = text.find(']', p + 1);
            if (close != std::string::npos && close > p + 1) {
                range = text.substr(p, close + 1 - p);
                p = skipSpaces(text, close + 1);
            }
        }
        size_t e = skipWordChars(text, p);
        if (e == p) { pos += 2; continue; }
        regs.push_back({kind, rangeWidth(trim(range), params),
                        text.substr(p, e - p)});
        pos = e;
    }
    return regs;
}

std::string jsonEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            case '\r': out += "\\r"; break;
            default: out += c;
        }
    }
    return out;
}

std::string widthJson(int width) {
    return width < 0 ? "null" : std::to_string(width);
}

}  // namespace

std::vector<Port> ModuleInfo::inputs() const {
    std::vector<Port> result;
    for (const auto& p : ports)
        if (p.dir == "input") result.push_back(p);
    return result;
}

ModuleInfo parseModule(const std::string& text, const std::string& top) {
    std::string clean = stripComments(text);
    std::string moduleText = findModule(clean, top);

    ModuleInfo info;
    info.top = top;
    info.params = parseParameters(moduleText);

    auto [header, body] = headerAndBody(moduleText);
    info.ports = parsePortDecls(header, info.params);
    if (info.ports.empty())  // non-ANSI style: directions in the body
        info.ports = parsePortDecls(body, info.params);

    info.freeRegs = parseFreeRegs(moduleText, info.params);
    info.hasAssume = findWord(moduleText, "assume", 0) != std::string::npos;
    return info;
}

std::string stripAssumes(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    const size_t n = text.size();
    size_t last = 0;
    size_t i = 0;
    while (true) {
        // find the next `\bassume\b(\s+property\b)?\s*\(`
        size_t start = std::string::npos, parenPos = std::string::npos;
        size_t search = i;
        while (search < n) {
            size_t hit = findWord(text, "assume", search);
            if (hit == std::string::npos) break;
            size_t j = hit + 6;
            size_t k = skipSpaces(text, j);
            if (k > j && wordAt(text, k, "property")) j = k + 8;
            j = skipSpaces(text, j);
            if (j < n && text[j] == '(') { start = hit; parenPos = j; break; }
            search = hit + 1;
        }
        if (start == std::string::npos) {
            out += text.substr(last);
            break;
        }
        out += text.substr(last, start - last);
        int depth = 1;
        size_t j = parenPos + 1;
        while (j < n && depth) {
            if (text[j] == '(') depth++;
            else if (text[j] == ')') depth--;
            j++;
        }
        while (j < n && (text[j] == ' ' || text[j] == '\t' || text[j] == '\n'))
            j++;
        if (j < n && text[j] == ';') j++;
        out += ';';  // null statement keeps `if (...) assume(x);` legal
        last = j;
        i = j;
    }
    return out;
}

std::string injectAssumes(const std::string& text, const std::string& top,
                          const std::vector<std::string>& assumes,
                          const std::string& clock) {
    if (assumes.empty()) return text;
    std::string body;
    for (size_t k = 0; k < assumes.size(); k++) {
        body += "        assume(" + assumes[k] + ");";
        if (k + 1 < assumes.size()) body += "\n";
    }
    std::string trigger = clock.empty() ? "@(*)" : "@(posedge " + clock + ")";
    std::string block =
        "\n    always " + trigger + " begin\n" + body + "\n    end\n";

    findModule(stripComments(text), top);  // throws when the module is absent

    size_t start = findModuleStart(text, top);
    if (start == std::string::npos)
        throw std::runtime_error("module '" + top + "' not found");
    size_t end = text.find("endmodule", start);
    if (end == std::string::npos)
        throw std::runtime_error("endmodule not found for module '" + top + "'");
    return text.substr(0, end) + block + text.substr(end);
}

std::optional<std::string> guessClock(const ModuleInfo& info,
                                      const std::string& configured) {
    if (!configured.empty()) return configured;
    for (const auto& cand : kClockNames)
        for (const auto& p : info.ports)
            if (p.dir == "input" && p.name == cand) return cand;
    return std::nullopt;
}

std::optional<ResetInfo> guessReset(const ModuleInfo& info,
                                    const std::optional<std::string>& clock,
                                    const std::optional<ResetInfo>& configured) {
    if (configured) return configured;
    for (const auto& cand : kResetNames) {
        if (clock && *clock == cand) continue;
        for (const auto& p : info.ports) {
            if (p.dir == "input" && p.name == cand) {
                // active low when the name (minus trailing 'i's) ends in 'n'
                std::string base = cand;
                while (!base.empty() && base.back() == 'i') base.pop_back();
                int active = (!base.empty() && base.back() == 'n') ? 0 : 1;
                return ResetInfo{cand, active, 2};
            }
        }
    }
    return std::nullopt;
}

std::string dumpJson(const ModuleInfo& info) {
    std::ostringstream os;
    os << "{";
    os << "\"top\": \"" << jsonEscape(info.top) << "\", ";
    os << "\"params\": {";
    bool first = true;
    for (const auto& [name, value] : info.params) {  // std::map: sorted keys
        if (!first) os << ", ";
        os << "\"" << jsonEscape(name) << "\": " << value;
        first = false;
    }
    os << "}, ";
    os << "\"ports\": [";
    first = true;
    for (const auto& p : info.ports) {
        if (!first) os << ", ";
        os << "{\"dir\": \"" << p.dir << "\", \"width\": " << widthJson(p.width)
           << ", \"name\": \"" << jsonEscape(p.name) << "\"}";
        first = false;
    }
    os << "], ";
    os << "\"free_regs\": [";
    first = true;
    for (const auto& r : info.freeRegs) {
        if (!first) os << ", ";
        os << "{\"kind\": \"" << r.kind << "\", \"width\": " << widthJson(r.width)
           << ", \"name\": \"" << jsonEscape(r.name) << "\"}";
        first = false;
    }
    os << "], ";
    os << "\"has_assume\": " << (info.hasAssume ? "true" : "false") << ", ";

    auto clock = guessClock(info);
    os << "\"clock\": ";
    if (clock) os << "\"" << jsonEscape(*clock) << "\"";
    else os << "null";
    os << ", ";

    auto reset = guessReset(info, clock);
    os << "\"reset\": ";
    if (reset)
        os << "{\"signal\": \"" << jsonEscape(reset->signal)
           << "\", \"active\": " << reset->active
           << ", \"cycles\": " << reset->cycles << "}";
    else
        os << "null";
    os << "}";
    return os.str();
}

}  // namespace frontend
}  // namespace smart
