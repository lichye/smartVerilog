#include "Mus.h"

#include <cvc5/cvc5.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <optional>
#include <fstream>
#include <functional>
#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace smart {
namespace mus {
namespace {

using Clock = std::chrono::steady_clock;

// ---------------------------------------------------------------------------
// Reading `.sl` define-funs
// ---------------------------------------------------------------------------
//
// A line looks like
//   (define-fun inv ((N1 Bool) (W (_ BitVec 8))) Bool (or N1 (bvult W ...)))
// utils.py pulls the parameter list and the body out with two regexes and then
// evaluates the body with a small s-expression interpreter. Same approach
// here, except the interpreter builds cvc5 Terms. Deliberately NOT std::regex
// over the file — only over one short line at a time (plan gotcha 18).

struct Definition {
    std::string line;                                  // the original text
    std::string body;                                  // the expression source
    std::vector<std::pair<std::string, int>> params;    // name, width (0 = Bool)
};

// Split "(a b (c d))" into a nested list of tokens.
struct SExpr {
    std::string atom;               // non-empty for a leaf
    std::vector<SExpr> list;
    bool isAtom() const { return !atom.empty(); }
};

std::vector<std::string> tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current;
    for (char c : text) {
        if (c == '(' || c == ')') {
            if (!current.empty()) { tokens.push_back(current); current.clear(); }
            tokens.emplace_back(1, c);
        } else if (std::isspace(static_cast<unsigned char>(c))) {
            if (!current.empty()) { tokens.push_back(current); current.clear(); }
        } else {
            current += c;
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

SExpr parseSExpr(const std::vector<std::string>& tokens, std::size_t& at) {
    if (at >= tokens.size()) throw std::runtime_error("unexpected end of expression");

    const std::string token = tokens[at++];
    if (token == "(") {
        SExpr node;
        while (at < tokens.size() && tokens[at] != ")") node.list.push_back(parseSExpr(tokens, at));
        if (at >= tokens.size()) throw std::runtime_error("unbalanced (");
        ++at;  // consume )
        return node;
    }
    if (token == ")") throw std::runtime_error("unbalanced )");

    SExpr leaf;
    leaf.atom = token;
    return leaf;
}

// Pull the parameter list and body out of one define-fun line.
std::optional<Definition> parseDefinition(const std::string& line) {
    const auto defineAt = line.find("define-fun");
    if (defineAt == std::string::npos) return std::nullopt;

    // The parameter list is the first parenthesised group after the name.
    const auto paramsOpen = line.find('(', defineAt);
    if (paramsOpen == std::string::npos) return std::nullopt;

    int depth = 0;
    std::size_t paramsClose = std::string::npos;
    for (std::size_t i = paramsOpen; i < line.size(); ++i) {
        if (line[i] == '(') ++depth;
        else if (line[i] == ')') {
            if (--depth == 0) { paramsClose = i; break; }
        }
    }
    if (paramsClose == std::string::npos) return std::nullopt;

    Definition definition;
    definition.line = line;

    const auto paramText = line.substr(paramsOpen, paramsClose - paramsOpen + 1);
    std::size_t at = 0;
    const auto tokens = tokenize(paramText);
    const SExpr params = parseSExpr(tokens, at);
    for (const auto& param : params.list) {
        if (param.isAtom() || param.list.empty()) continue;
        const std::string name = param.list[0].atom;
        int width = 0;  // Bool
        if (param.list.size() >= 2 && !param.list[1].isAtom()) {
            // (_ BitVec N)
            const auto& sort = param.list[1].list;
            if (sort.size() == 3 && sort[1].atom == "BitVec") width = std::stoi(sort[2].atom);
        }
        definition.params.emplace_back(name, width);
    }

    // The body is what follows the return sort. The return sort is either
    // `Bool` or `(_ BitVec N)`.
    std::size_t bodyStart = paramsClose + 1;
    while (bodyStart < line.size() && std::isspace(static_cast<unsigned char>(line[bodyStart])))
        ++bodyStart;
    if (line.compare(bodyStart, 4, "Bool") == 0) {
        bodyStart += 4;
    } else if (bodyStart < line.size() && line[bodyStart] == '(') {
        int sortDepth = 0;
        std::size_t i = bodyStart;
        for (; i < line.size(); ++i) {
            if (line[i] == '(') ++sortDepth;
            else if (line[i] == ')' && --sortDepth == 0) { ++i; break; }
        }
        bodyStart = i;
    }
    while (bodyStart < line.size() && std::isspace(static_cast<unsigned char>(line[bodyStart])))
        ++bodyStart;

    // Drop the closing paren of the define-fun itself.
    std::size_t bodyEnd = line.find_last_of(')');
    if (bodyEnd == std::string::npos || bodyEnd <= bodyStart) return std::nullopt;
    definition.body = line.substr(bodyStart, bodyEnd - bodyStart);
    if (definition.body.empty()) return std::nullopt;
    return definition;
}

// ---------------------------------------------------------------------------
// s-expression -> cvc5::Term
// ---------------------------------------------------------------------------

class TermBuilder {
public:
    explicit TermBuilder(cvc5::TermManager& tm) : tm_(tm) {}

    cvc5::Term variable(const std::string& name, int width) {
        auto found = variables_.find(name);
        if (found != variables_.end()) return found->second;
        const cvc5::Sort sort =
            width == 0 ? tm_.getBooleanSort() : tm_.mkBitVectorSort(width);
        const cvc5::Term term = tm_.mkConst(sort, name);
        variables_.emplace(name, term);
        return term;
    }

    const std::map<std::string, cvc5::Term>& variables() const { return variables_; }

    cvc5::Term build(const SExpr& node) {
        if (node.isAtom()) {
            if (node.atom == "true") return tm_.mkTrue();
            if (node.atom == "false") return tm_.mkFalse();
            auto found = variables_.find(node.atom);
            if (found != variables_.end()) return found->second;
            // An undeclared symbol in a body is a Bool, as in utils.py.
            return variable(node.atom, 0);
        }

        if (node.list.empty()) throw std::runtime_error("empty expression");

        // (_ bvN W)
        if (node.list[0].atom == "_" && node.list.size() == 3 &&
            node.list[1].atom.rfind("bv", 0) == 0) {
            const auto value = node.list[1].atom.substr(2);
            const int width = std::stoi(node.list[2].atom);
            return tm_.mkBitVector(width, value, 10);
        }

        const std::string op = node.list[0].atom;
        std::vector<cvc5::Term> args;
        for (std::size_t i = 1; i < node.list.size(); ++i)
            args.push_back(build(node.list[i]));

        static const std::map<std::string, cvc5::Kind> kinds = {
            {"and", cvc5::Kind::AND},       {"or", cvc5::Kind::OR},
            {"not", cvc5::Kind::NOT},       {"xor", cvc5::Kind::XOR},
            {"=>", cvc5::Kind::IMPLIES},    {"ite", cvc5::Kind::ITE},
            {"=", cvc5::Kind::EQUAL},       {"distinct", cvc5::Kind::DISTINCT},
            {"bvnot", cvc5::Kind::BITVECTOR_NOT},
            {"bvand", cvc5::Kind::BITVECTOR_AND},
            {"bvor", cvc5::Kind::BITVECTOR_OR},
            {"bvxor", cvc5::Kind::BITVECTOR_XOR},
            {"bvadd", cvc5::Kind::BITVECTOR_ADD},
            {"bvsub", cvc5::Kind::BITVECTOR_SUB},
            {"bvmul", cvc5::Kind::BITVECTOR_MULT},
            {"bvneg", cvc5::Kind::BITVECTOR_NEG},
            {"bvudiv", cvc5::Kind::BITVECTOR_UDIV},
            {"bvurem", cvc5::Kind::BITVECTOR_UREM},
            {"bvshl", cvc5::Kind::BITVECTOR_SHL},
            {"bvlshr", cvc5::Kind::BITVECTOR_LSHR},
            {"bvashr", cvc5::Kind::BITVECTOR_ASHR},
            {"concat", cvc5::Kind::BITVECTOR_CONCAT},
            {"bvult", cvc5::Kind::BITVECTOR_ULT},
            {"bvule", cvc5::Kind::BITVECTOR_ULE},
            {"bvugt", cvc5::Kind::BITVECTOR_UGT},
            {"bvuge", cvc5::Kind::BITVECTOR_UGE},
            {"bvslt", cvc5::Kind::BITVECTOR_SLT},
            {"bvsle", cvc5::Kind::BITVECTOR_SLE},
            {"bvsgt", cvc5::Kind::BITVECTOR_SGT},
            {"bvsge", cvc5::Kind::BITVECTOR_SGE},
        };

        const auto kind = kinds.find(op);
        if (kind == kinds.end())
            throw std::runtime_error("unsupported operator in .sl body: " + op);
        return tm_.mkTerm(kind->second, args);
    }

private:
    cvc5::TermManager& tm_;
    std::map<std::string, cvc5::Term> variables_;
};

std::set<std::string> freeVariables(const cvc5::Term& term) {
    std::set<std::string> names;
    std::function<void(const cvc5::Term&)> walk = [&](const cvc5::Term& node) {
        if (node.getNumChildren() == 0) {
            if (node.getKind() == cvc5::Kind::CONSTANT) names.insert(node.toString());
            return;
        }
        for (std::size_t i = 0; i < node.getNumChildren(); ++i) walk(node[i]);
    };
    walk(term);
    return names;
}

// ---------------------------------------------------------------------------
// Independent groups (utils.DisjointSets)
// ---------------------------------------------------------------------------

std::vector<std::vector<std::size_t>> independentGroups(
    const std::vector<std::set<std::string>>& variablesPer) {
    const std::size_t n = variablesPer.size();
    std::vector<std::size_t> parent(n);
    std::iota(parent.begin(), parent.end(), 0);

    std::function<std::size_t(std::size_t)> find = [&](std::size_t i) {
        while (parent[i] != i) { parent[i] = parent[parent[i]]; i = parent[i]; }
        return i;
    };
    const auto unite = [&](std::size_t a, std::size_t b) {
        a = find(a); b = find(b);
        if (a != b) parent[b] = a;
    };

    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = i + 1; j < n; ++j) {
            const auto& left = variablesPer[i];
            const auto& right = variablesPer[j];
            if (std::any_of(left.begin(), left.end(),
                            [&](const std::string& v) { return right.count(v) != 0; }))
                unite(i, j);
        }

    std::map<std::size_t, std::vector<std::size_t>> groups;
    for (std::size_t i = 0; i < n; ++i) groups[find(i)].push_back(i);

    std::vector<std::vector<std::size_t>> result;
    result.reserve(groups.size());
    for (auto& [root, members] : groups) result.push_back(std::move(members));
    return result;
}

// Greedy hitting set (utils.approx_mhs): repeatedly take the variable that
// covers the most still-uncovered assertions.
std::set<std::string> approxMinimalHittingSet(
    const std::vector<std::set<std::string>>& sets) {
    std::set<std::string> hitting;
    std::vector<const std::set<std::string>*> uncovered;
    for (const auto& s : sets)
        if (!s.empty()) uncovered.push_back(&s);

    while (!uncovered.empty()) {
        std::map<std::string, int> counts;
        for (const auto* s : uncovered)
            for (const auto& v : *s) ++counts[v];
        if (counts.empty()) break;

        const auto best = std::max_element(
            counts.begin(), counts.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });
        hitting.insert(best->first);

        std::vector<const std::set<std::string>*> stillUncovered;
        for (const auto* s : uncovered)
            if (s->count(best->first) == 0) stillUncovered.push_back(s);
        uncovered.swap(stillUncovered);
    }
    return hitting;
}

// ---------------------------------------------------------------------------
// MSA search
// ---------------------------------------------------------------------------

struct MusCheck {
    bool isMus = false;
    std::map<std::string, std::string> assignments;  // when isMus
    std::vector<std::size_t> core;                   // when !isMus
};

// Is `candidate` a set of variables we can universally quantify away and still
// satisfy every assertion? Assumption tags give us the unsat core when not.
MusCheck isMus(cvc5::TermManager& tm, const std::vector<cvc5::Term>& assertions,
               const std::map<std::string, cvc5::Term>& variables,
               const std::set<std::string>& candidate) {
    MusCheck result;
    if (candidate.empty()) {
        result.isMus = true;
        return result;
    }

    cvc5::Solver solver(tm);
    solver.setOption("produce-models", "true");
    solver.setOption("produce-unsat-cores", "true");
    solver.setOption("minimal-unsat-cores", "true");

    std::vector<cvc5::Term> bound;
    for (const auto& name : candidate) {
        const auto found = variables.find(name);
        if (found == variables.end()) continue;
        bound.push_back(tm.mkVar(found->second.getSort(), name));
    }

    std::vector<cvc5::Term> tags;
    for (std::size_t i = 0; i < assertions.size(); ++i) {
        cvc5::Term body = assertions[i];
        if (!bound.empty()) {
            // Rebuild the assertion over bound variables, then quantify.
            std::vector<cvc5::Term> from, to;
            std::size_t at = 0;
            for (const auto& name : candidate) {
                const auto found = variables.find(name);
                if (found == variables.end()) continue;
                from.push_back(found->second);
                to.push_back(bound[at++]);
            }
            body = body.substitute(from, to);
            body = tm.mkTerm(cvc5::Kind::FORALL,
                             {tm.mkTerm(cvc5::Kind::VARIABLE_LIST, bound), body});
        }
        const cvc5::Term tag = tm.mkConst(tm.getBooleanSort(), "@a" + std::to_string(i));
        tags.push_back(tag);
        solver.assertFormula(tm.mkTerm(cvc5::Kind::IMPLIES, {tag, body}));
    }

    const cvc5::Result answer = solver.checkSatAssuming(tags);
    if (answer.isSat()) {
        result.isMus = true;
        for (const auto& [name, term] : variables) {
            try {
                result.assignments[name] = solver.getValue(term).toString();
            } catch (const std::exception&) {
                // A variable the solver did not need to fix.
            }
        }
        return result;
    }

    result.isMus = false;
    if (answer.isUnsat()) {
        const auto core = solver.getUnsatCore();
        for (std::size_t i = 0; i < tags.size(); ++i)
            if (std::find(core.begin(), core.end(), tags[i]) != core.end())
                result.core.push_back(i);
    }
    if (result.core.empty())  // unknown, or an empty core: treat all as relevant
        for (std::size_t i = 0; i < assertions.size(); ++i) result.core.push_back(i);
    return result;
}

std::set<std::string> minimalWitnesses(
    const std::set<std::string>& msa,
    const std::vector<std::set<std::string>>& coreVariables) {
    // The independent variable sets of the core, minus what we already fixed.
    const auto groups = independentGroups(coreVariables);
    std::vector<std::set<std::string>> reduced;
    for (const auto& group : groups) {
        std::set<std::string> merged;
        for (auto index : group)
            merged.insert(coreVariables[index].begin(), coreVariables[index].end());
        std::set<std::string> difference;
        std::set_difference(merged.begin(), merged.end(), msa.begin(), msa.end(),
                            std::inserter(difference, difference.end()));
        if (!difference.empty()) reduced.push_back(std::move(difference));
    }
    return approxMinimalHittingSet(reduced);
}

std::set<std::string> difference(const std::set<std::string>& a,
                                 const std::set<std::string>& b) {
    std::set<std::string> result;
    std::set_difference(a.begin(), a.end(), b.begin(), b.end(),
                        std::inserter(result, result.end()));
    return result;
}

}  // namespace

MusResult getMus(const std::string& variablesFile, const std::string& sygusResultFile,
                 int timeoutSeconds) {
    MusResult result;

    std::set<std::string> variables;
    {
        std::ifstream in(variablesFile);
        if (!in) throw std::runtime_error("cannot read " + variablesFile);
        std::string name;
        while (in >> name)
            if (!name.empty()) variables.insert(name);
    }

    std::vector<Definition> definitions;
    {
        std::ifstream in(sygusResultFile);
        if (!in) throw std::runtime_error("cannot read " + sygusResultFile);
        std::string line;
        while (std::getline(in, line))
            if (auto definition = parseDefinition(line)) definitions.push_back(*definition);
    }
    if (definitions.empty()) {
        result.underspecified = variables;
        return result;
    }

    cvc5::TermManager tm;
    TermBuilder builder(tm);

    std::vector<cvc5::Term> assertions;
    std::vector<std::set<std::string>> variablesPer;
    for (const auto& definition : definitions) {
        for (const auto& [name, width] : definition.params) builder.variable(name, width);
        try {
            std::size_t at = 0;
            const auto tokens = tokenize(definition.body);
            const SExpr expression = parseSExpr(tokens, at);
            const cvc5::Term term = builder.build(expression);
            assertions.push_back(term);
            variablesPer.push_back(freeVariables(term));
        } catch (const std::exception&) {
            // A body we cannot rebuild is skipped rather than sinking the run;
            // it just does not constrain the MSA.
        }
    }
    if (assertions.empty()) {
        result.underspecified = variables;
        return result;
    }

    const auto groups = independentGroups(variablesPer);
    result.groups = static_cast<int>(groups.size());

    const auto deadline = Clock::now() + std::chrono::seconds(timeoutSeconds);
    std::set<std::string> msa;

    for (const auto& group : groups) {
        std::vector<cvc5::Term> groupAssertions;
        std::vector<std::set<std::string>> groupVariablesPer;
        std::set<std::string> allVariables;
        for (auto index : group) {
            groupAssertions.push_back(assertions[index]);
            groupVariablesPer.push_back(variablesPer[index]);
            allVariables.insert(variablesPer[index].begin(), variablesPer[index].end());
        }

        // A single assertion over a single variable pins that variable down.
        if (groupAssertions.size() == 1 && groupVariablesPer[0].size() == 1) {
            msa.insert(allVariables.begin(), allVariables.end());
            continue;
        }

        std::set<std::string> candidate = approxMinimalHittingSet(groupVariablesPer);
        auto check = isMus(tm, groupAssertions, builder.variables(),
                           difference(allVariables, candidate));

        while (!check.isMus) {
            if (Clock::now() >= deadline) {
                result.timedOut = true;
                break;
            }
            std::vector<std::set<std::string>> coreVariables;
            for (auto index : check.core)
                if (index < groupVariablesPer.size())
                    coreVariables.push_back(groupVariablesPer[index]);

            const auto witnesses = minimalWitnesses(candidate, coreVariables);
            if (witnesses.empty()) break;  // no progress possible
            candidate.insert(witnesses.begin(), witnesses.end());
            check = isMus(tm, groupAssertions, builder.variables(),
                          difference(allVariables, candidate));
        }

        msa.insert(candidate.begin(), candidate.end());
        for (const auto& [name, value] : check.assignments)
            result.assignments[name] = value;
    }

    result.underspecified = difference(variables, msa);
    return result;
}

MinimiseResult minimiseAssertions(const std::string& inSl, const std::string& outSl,
                                  int timeoutSeconds) {
    MinimiseResult result;

    std::vector<Definition> definitions;
    {
        std::ifstream in(inSl);
        if (!in) throw std::runtime_error("cannot read " + inSl);
        std::string line;
        while (std::getline(in, line))
            if (auto definition = parseDefinition(line)) definitions.push_back(*definition);
    }
    result.before = definitions.size();
    if (definitions.size() <= 1) {
        for (const auto& definition : definitions)
            result.keptDefinitions.push_back(definition.line);
        result.after = result.keptDefinitions.size();
        std::ofstream out(outSl);
        for (const auto& line : result.keptDefinitions) out << line << "\n";
        return result;
    }

    cvc5::TermManager tm;
    TermBuilder builder(tm);

    std::vector<cvc5::Term> terms;
    std::vector<std::size_t> keep;
    std::vector<std::set<std::string>> variablesPer;
    for (std::size_t i = 0; i < definitions.size(); ++i) {
        for (const auto& [name, width] : definitions[i].params) builder.variable(name, width);
        try {
            std::size_t at = 0;
            const auto tokens = tokenize(definitions[i].body);
            const SExpr expression = parseSExpr(tokens, at);
            terms.push_back(builder.build(expression));
            variablesPer.push_back(freeVariables(terms.back()));
            keep.push_back(i);
        } catch (const std::exception&) {
            // Unparseable bodies are kept verbatim: dropping an assertion we
            // do not understand would silently weaken the result.
            result.keptDefinitions.push_back(definitions[i].line);
        }
    }

    // PowerLattice: try to drop one assertion at a time, cheapest first, and
    // keep the drop when what remains still implies everything.
    std::vector<std::size_t> order(keep.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
        return terms[a].getNumChildren() < terms[b].getNumChildren();
    });

    std::vector<bool> kept(keep.size(), true);
    const auto deadline = Clock::now() + std::chrono::seconds(timeoutSeconds);

    cvc5::Solver solver(tm);
    const auto impliesRest = [&](std::size_t dropped) {
        std::vector<cvc5::Term> remaining;
        for (std::size_t i = 0; i < kept.size(); ++i)
            if (kept[i] && i != dropped) remaining.push_back(terms[i]);
        if (remaining.empty()) return false;

        solver.push();
        for (const auto& term : remaining) solver.assertFormula(term);
        solver.assertFormula(tm.mkTerm(cvc5::Kind::NOT, {terms[dropped]}));
        const cvc5::Result answer = solver.checkSat();
        solver.pop();
        return answer.isUnsat();
    };

    for (auto index : order) {
        if (Clock::now() >= deadline) break;
        if (!kept[index]) continue;
        try {
            if (impliesRest(index)) kept[index] = false;
        } catch (const std::exception&) {
            // Keep anything the solver chokes on.
        }
    }

    for (std::size_t i = 0; i < kept.size(); ++i)
        if (kept[i]) result.keptDefinitions.push_back(definitions[keep[i]].line);

    result.after = result.keptDefinitions.size();

    std::ofstream out(outSl);
    if (!out) throw std::runtime_error("cannot write " + outSl);
    for (const auto& line : result.keptDefinitions) out << line << "\n";
    return result;
}

}  // namespace mus
}  // namespace smart
