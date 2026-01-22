import time
import sys
import os
from collections import defaultdict
from cvc5.pythonic import *

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "src", "python")))
from utils import *

seen_mus = []
seen_notmus = []

solver = Solver()
solver.setOption("produce-models", "true")
solver.set("produce-unsat-cores", "true")
solver.set("minimal-unsat-cores", "true")

def get_mus(v_file, a_file, timeout):
    global seen_mus, seen_notmus
    seen_mus, seen_notmus = [], []

    iassertions, iall_vars_inq, ivars_per_assertion = get_assertions(a_file)
    variables = get_variables(v_file)

    msa = set()
    mus = set()
    assignments = {}
    for assertions, all_vars_inq, vars_per_assertion in zip(iassertions, iall_vars_inq, ivars_per_assertion):
        if len(vars_per_assertion) == 1 and len(vars_per_assertion[0]) == 1:
            n = all_vars_inq
            best_msa = set(map(str, n))
            amus = set()

            s = Solver()
            s.add(assertions)
            s.check()
            ass = {str(d): s.model()[d].as_ast().toPythonObj() for d in s.model().decls()}
        else:
            n, ass = search_msa(assertions, vars_per_assertion, all_vars_inq, timeout)
            amus = all_vars_inq - n
            best_msa = set(map(str, n))

        msa.update(best_msa)
        mus.update(amus)
        if ass:
            assignments.update(ass)

    return variables - msa, assignments

def search_msa(assertions, vars_per_assertion, all_vars, timeout):
    mhs = approx_mhs(vars_per_assertion)
    match is_mus(all_vars - mhs, assertions):
        case [True, model]:
            return mhs, model
        case [False, ucore]:
            return ascend_to_boundary(mhs, all_vars, assertions, ucore, timeout)

def most_frequent_var(vars_per_assertion):
    counter = defaultdict(int)
    for s in vars_per_assertion:
        for v in s:
            counter[v] +=1
    return max(counter, key=counter.get)

def approx_mhs(vars_per_assertion, *, T=None):
    T = T or set()
    uncovered = {x for x in vars_per_assertion if not x & T}
    T.add(most_frequent_var(uncovered))
    while uncovered:
        nv = most_frequent_var(uncovered)
        T.add(nv)
        uncovered = {x for x in uncovered if nv not in x}
    return T

def is_mus(cand, assertions):
    global seen_mus, seen_notmus
    if not cand:
        return True, None

    if any(cand.issubset(m) for m in seen_mus):
        return True, None
    if any(n.issubset(cand) for n in seen_notmus):
        return False, None

    solver.push()
    orig_as = list(assertions.children())
    tags = [Bool(f"@a{i}") for i in range(len(orig_as))]

    for t, a in zip(tags, orig_as):
        solver.add(Implies(t, ForAll(list(cand), a)))

    if solver.check(tags) == sat:
        seen_mus = {m for m in seen_mus if not m.issubset(cand)}
        seen_mus.add(frozenset(cand))
        model = solver.model()
        rds = [d for d in model.decls() if str(d)[0] != "@"]
        assignments = {str(d): model[d].as_ast().toPythonObj() for d in rds}
        solver.pop()
        return True, assignments
    else:
        seen_notmus = {n for n in seen_notmus if not cand.issubset(n)}
        seen_notmus.add(frozenset(cand))
        core_tags = set(solver.unsat_core())
        pas = [a for t, a in zip(tags, orig_as) if t in core_tags]
        solver.pop()
        return False, pas

def ascend_to_boundary(mhs ,all_vars, assertions, pas, timeout):
    start = time.time()
    minimal_vars = minimal_witnesses(mhs, pas)
    msa = mhs.union(minimal_vars)
    while not (ret := is_mus(all_vars - msa, assertions))[0]:
        if time.time() - start >= timeout:
            return msa, {}
        w = minimal_witnesses(msa, ret[1])
        msa.update(w)
    return msa, ret[1]

def minimal_witnesses(msa, pas):
    d = DisjointSets(pas)
    rvs = [d.difference(msa) for d in d.var_ind_subsets()]
    return approx_mhs(rvs)


