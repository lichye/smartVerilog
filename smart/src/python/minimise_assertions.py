import re
import time
from itertools import combinations

from cvc5.pythonic import *
from utils import *


def_pat = re.compile(r'.*define-fun.*')
func_pat = re.compile(r'(?:Bool|\(_\s+BitVec\s+\d+\))\s+(.*)\)')
var_pat = re.compile(r'\((\w+)\s+(?:(Bool)|\(_\s+(BitVec)\s+(\d+)\))\)')


class PowerLattice:
    def __init__(self, base, timeout):
        self.timeout = timeout
        self.base = self.sort_by_complexity(base)

        self._base = frozenset(range(len(self.base)))
        self.curr_best = (self._base, len(self.base))
        self.tried = set()
        self.l = len(self.base)
        self.rel_lattice = self.generate_rel_lattice(self._base, self.l)

        self.neg = [Not(r) for r in self.base]
        self.s = Solver()

    @staticmethod
    def sort_by_complexity(base):
        return list(sorted(base, key=lambda x: x.num_args()))

    def generate_rel_lattice(self, A, l):
        for t in combinations(A, l - 1):
            yield frozenset(t)

    def _minimise(self):
        start = time.time()
        while True:
            if time.time() - start >= self.timeout:
                return

            curr_best, l = self.curr_best
            level = l - 1
            if level != self.l:
                self.rel_lattice = self.generate_rel_lattice(curr_best, l)
                self.l = level

            next_can = None
            for r in self.rel_lattice:
                if not any(r.issubset(t) for t in self.tried):
                    next_can = r
                    break
            if next_can is None:
                return

            next_can_ass_idx = next_can
            rst_idx = self._base - next_can

            if self.test_can_implies_rest(next_can_ass_idx, rst_idx):
                self.curr_best = (next_can, level)
            else:
                self.tried.add(next_can)

    def ensure_equal(self):
        best_idx = self.curr_best[0]
        removed_idx = self._base - best_idx

        assert self.test_can_implies_rest(best_idx, removed_idx), \
            "Minimal doesn't imply removed constraints"

        for kept_idx in best_idx:
            test_remove = best_idx - {kept_idx}
            if self.test_can_implies_rest(test_remove, {kept_idx}):
                print(f"ERROR: Constraint {kept_idx} is redundant but kept!")
                assert False

    def minimise(self):
        self._minimise()
        # self.ensure_equal()
        return [self.base[i] for i in self.curr_best[0]]

    def test_can_implies_rest(self, A_idx, B_idx):
        if not A_idx: return True 
        if not B_idx: return True  
        
        A_list   = [self.base[i] for i in A_idx]
        notBlist = [self.neg[i]  for i in B_idx]
        
        self.s.push()
        self.s.add(A_list[0] if len(A_list) == 1 else And(*A_list))
        self.s.add(notBlist[0] if len(notBlist) == 1 else Or(*notBlist))
        r = self.s.check()
        self.s.pop()
        return r == unsat

def minimise(assertions, timeout):
    min_core = []
    for p in assertions:
        p = set(p.children())
        if len(p) == 1:
            ass ,= p
            min_core.append(ass.sexpr())
            continue

        pl = PowerLattice(p, timeout)
        sub_min_core = pl.minimise()
        min_core.extend([x.sexpr() for x in sub_min_core])
        del pl
    return min_core

def run_minimisation(in_file, out_file, timeout):
    iassertions, deffuns = get_assertions_mini(in_file)
    min_core = minimise(iassertions, timeout)

    with open(out_file, 'w') as f:
        for mc in min_core:
            df = f"\n{deffuns[mc]}"
            f.write(df)

    return min_core, deffuns
