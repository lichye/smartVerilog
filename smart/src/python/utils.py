import re 
from cvc5.pythonic import *
from itertools import chain
from more_itertools import map_reduce, bucket, collapse, partition


def get_assertions(file):
    func_pat = re.compile(r"(?:Bool|\(_\s+BitVec\s+\d+\))\s+(.*)\)")
    var_pat = re.compile(r"\((\w+)\s+(?:(Bool)|\(_\s+(BitVec)\s+(\d+)\))\)")

    assertions = set()
    env = {}
 
    with open(file, 'r') as f:
        for l in f.readlines():
            if (body := func_pat.search(l)) is not None:
                expr = body.group(1)
                for m in var_pat.findall(l):
                    match m:
                        case [n, "", _, w]:  env[n] = BitVec(n, int(w))
                        case [n, _, "", ""]: env[n] = Bool(n)
                assertions.add(parse_body_to_cvc5(expr, env=env))

    indp_assertions = DisjointSets(assertions).assertion_ind_subsets()
    iassertions = [And(*ias) if len(ias) > 1 else And(*ias, *ias) for ias in indp_assertions]
    ifree_vars = [frozenset(free_variables(ias)) for ias in iassertions]
    ivar_per_ass = [[frozenset(free_variables(a)) for a in ias] for ias in indp_assertions]
    return iassertions, ifree_vars, ivar_per_ass

def get_assertions_mini(file):
    func_pat = re.compile(r"(?:Bool|\(_\s+BitVec\s+\d+\))\s+(.*)\)")
    var_pat = re.compile(r"\((\w+)\s+(?:(Bool)|\(_\s+(BitVec)\s+(\d+)\))\)")

    assertions = set()
    deffuns = {}
    env = {}

    with open(file, 'r') as f:
        for l in f.readlines():
            if (body := func_pat.search(l)) is not None:
                expr = body.group(1)
                for m in var_pat.findall(l):
                    match m:
                        case [n, "", _, w]:  env[n] = BitVec(n, int(w))
                        case [n, _, "", ""]: env[n] = Bool(n)
                deffuns[expr] = l
                assertions.add(parse_body_to_cvc5(expr, env=env))

    indp_assertions = DisjointSets(assertions).assertion_ind_subsets()
    iassertions = [And(*ias) if len(ias) > 1 else And(*ias, *ias) for ias in indp_assertions]
    return iassertions, deffuns

def _tok(s: str):
    s = s.replace('(', ' ( ').replace(')', ' ) ')
    return [t for t in s.split() if t]

def _parse(tokens):
    t = tokens.pop(0)
    if t == '(':
        lst = []
        while tokens[0] != ')':
            lst.append(_parse(tokens))
        tokens.pop(0)
        return lst
    elif t == ')':
        raise ValueError('unmatched )')
    else:
        return t

def _eval(ast, env, bv_widths):
    if isinstance(ast, str):
        if ast == 'true':  return BoolVal(True)
        if ast == 'false': return BoolVal(False)
        # use declared symbol if present; otherwise assume Bool
        if ast in env:
            return env[ast]
        v = Bool(ast)
        env[ast] = v
        return v

    if isinstance(ast, list) and len(ast) == 3 and ast[0] == '_' and str(ast[1]).startswith('bv'):
        val = int(str(ast[1])[2:]); width = int(ast[2])
        return BitVecVal(val, width)

    op, *args = ast
    if op == 'and':  return And(*[_eval(a, env, bv_widths) for a in args])
    if op == 'or':   return Or(*[_eval(a, env, bv_widths) for a in args])
    if op == 'not':  return Not(_eval(args[0], env, bv_widths))
    if op == 'xor':  return Xor(_eval(args[0], env, bv_widths), _eval(args[1], env, bv_widths))
    if op == '=>':   return Implies(_eval(args[0], env, bv_widths), _eval(args[1], env, bv_widths))
    if op == 'ite':
        c,a,b = (_eval(x, env, bv_widths) for x in args)
        return If(c,a,b)
    if op == '=':
        a = _eval(args[0], env, bv_widths); b = _eval(args[1], env, bv_widths)
        return a == b
    # simple BV ops (extend as needed)
    if op == 'bvnot': return ~_eval(args[0], env, bv_widths)
    if op == 'bvand': return _eval(args[0], env, bv_widths) & _eval(args[1], env, bv_widths)
    if op == 'bvor':  return _eval(args[0], env, bv_widths) | _eval(args[1], env, bv_widths)
    if op == 'bvxor': return _eval(args[0], env, bv_widths) ^ _eval(args[1], env, bv_widths)
    if op == 'bvadd': return _eval(args[0], env, bv_widths) + _eval(args[1], env, bv_widths)
    if op == 'bvsub': return _eval(args[0], env, bv_widths) - _eval(args[1], env, bv_widths)
    if op == 'bvmul': return _eval(args[0], env, bv_widths) * _eval(args[1], env, bv_widths)
    if op == 'bvneg': return -_eval(args[0], env, bv_widths)
    if op == 'concat': return Concat(*[_eval(a, env, bv_widths) for a in args])
    if op == 'bvult':
        a,b = _eval(args[0], env, bv_widths), _eval(args[1], env, bv_widths)
        return ULT(a,b)
    if op == 'bvule':
        a,b = _eval(args[0], env, bv_widths), _eval(args[1], env, bv_widths)
        return ULE(a,b)
    if op == 'bvsle':
        a,b = _eval(args[0], env, bv_widths), _eval(args[1], env, bv_widths)
        return SLE(a,b)
    if op == 'bvugt':
        a,b = _eval(args[0], env, bv_widths), _eval(args[1], env, bv_widths)
        return UGT(a,b)
    if op == 'bvuge':
        a,b = _eval(args[0], env, bv_widths), _eval(args[1], env, bv_widths)
        return UGE(a,b)

    # unknown operator -> treat as symbol (rare in bodies)
    return env[op]

def parse_body_to_cvc5(body: str, *, env, bv_widths=None):
    tokens = _tok(body)
    ast = _parse(tokens)
    return _eval(ast, env, bv_widths or {})

def free_variables(a):
    vs = set()
    stack = [a]
    while stack:
        t = stack.pop()
        if (cs := t.children()):
            stack.extend(cs)
        else:
            vs.add(t)
    return frozenset(vs)

def get_variables(file):
    with open(file, "r") as f: 
        variables = set(v for v in f.read().split())
    return variables


class DisjointSets:
    def __init__(self, A):
        self._elems = map_reduce(A, free_variables)
        self._parents = {fv: fv for fv in self._elems}
        self._ranks = {fv: 0 for fv in self._elems}
        self._merge_overlaps()

    def _find(self, i):
        if self._parents[i] != i:
            self._parents[i] = self._find(self._parents[i])
        return self._parents[i]

    def _union(self, i, j):
        ir, jr = self._find(i), self._find(j)
        if ir == jr:
            return
        if self._ranks[ir] < self._ranks[jr]:
            self._parents[ir] = jr
        elif self._ranks[ir] > self._ranks[jr]:
            self._parents[jr] = ir
        else:
            self._parents[jr] = ir
            self._ranks[ir] += 1

    def _merge_overlaps(self):
        fvs = list(self._elems)
        for i in range(len(fvs)):
            for j in range(i + 1, len(fvs)):
                if fvs[i] & fvs[j]:
                    self._union(fvs[i], fvs[j])

    def assertion_ind_subsets(self):
        parent_map = {fv: self._find(fv) for fv in self._elems}
        grouped = bucket(self._elems.items(), key=lambda kv: parent_map[kv[0]])
        return [list(collapse([v for _, v in grouped[group]])) for group in grouped]

    def var_ind_subsets(self):
        parent_map = {fv: self._find(fv) for fv in self._elems}
        grouped = bucket(parent_map.keys(), key=lambda fv: parent_map[fv])
        return [frozenset(collapse(grouped[group])) for group in grouped]
