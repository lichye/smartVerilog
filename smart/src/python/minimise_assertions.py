# Author: Guy Frankel
import re
import z3
import igraph as ig

import multiprocessing as mp
import os
import matplotlib.pyplot as plt
import numpy as np
import gc
import traceback

from itertools import combinations, chain
from collections import defaultdict
import random

def_pat = re.compile(r'.*define-fun.*')
func_pat = re.compile(r'(?:Bool|\(_\s+BitVec\s+\d+\))\s+(.*)\)')
var_pat = re.compile(r'\((\w+)\s+(?:(Bool)|\(_\s+(BitVec)\s+(\d+)\))\)')


import z3
from itertools import combinations

# Saving each inverse (base - candidate) combination is memory heavy
#  -> On the bigger benchrmarks with ~7000 assertions
#     there exist 7000!/(3!(7000 - 3)!) ~ 57,142,169,000 combinations with three assertions removed 
class CheckedCombDataStruct:
    ...

class PowerLattice:
    def __init__(self, funs):
        self.decls = {}
        defined_funs, base = funs
        self.base = list(base)
        self.generate_vars(defined_funs)

        self._base = frozenset(range(len(self.base)))
        self.curr_best = (self._base, len(self.base))
        self.tried = set()
        self.l = len(self.base)
        self.rel_lattice = self.generate_rel_lattice(self._base, self.l)

        self.base_refs = [z3.parse_smt2_string(f"(assert {b})", decls=self.decls)[0]
                        for b in self.base]
        self.neg_refs = [z3.Not(r) for r in self.base_refs]
        self.s = z3.Solver()

    def generate_vars(self, exprs):
        for expr in exprs:
            for m in var_pat.findall(expr):
                match m:
                    case [n, '', _, w]:
                        self.decls[n] = z3.BitVec(n, int(w))
                    case [n, _, '', '']:
                        self.decls[n] = z3.Bool(n)

    def generate_rel_lattice(self, A, l):
        for t in combinations(A, l - 1):
            yield frozenset(t)

    def _minimise(self):
        while True:
            curr_best, l = self.curr_best
            print("Current best: ", l)
            level = l - 1
            if level != self.l:
                # self.tried = set() # I think that each time we move to a new layer we can reset the tried set
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

            if self.test_can_refs(next_can_ass_idx, rst_idx):
                self.curr_best = (next_can, level)
            else:
                self.tried.add(next_can)


    def ensure_equal(self):
        best_idx = self.curr_best[0]
        best_refs = [self.base_refs[i] for i in best_idx]
        self.s.push()
        self.s.add(z3.And(self.base_refs) == z3.And(best_refs))
        assert self.s.check() == z3.sat
        self.s.pop()

    def minimise(self):
        self._minimise()
        self.ensure_equal()
        return [self.base[i] for i in self.curr_best[0]]


    def test_can_refs(self, A_idx, B_idx):
        # A_idx, B_idx are iterables of indices (e.g., frozenset[int])
        A_list   = [self.base_refs[i] for i in A_idx]
        notBlist = [self.neg_refs[i]  for i in B_idx]

        self.s.push()
        if A_list:
            self.s.add(z3.And(A_list))
        if notBlist:
            self.s.add(z3.Or(notBlist))
        else:
            self.s.pop()
            return True

        r = self.s.check()
        self.s.pop()
        return r == z3.unsat


def get_defined_funs(file):
    with open(file, 'r') as f:
        content = f.read()
        matches = def_pat.findall(content)
    return set(matches)

def get_unique_funs(defined_funs):
    pair = ([], [])

    for df in defined_funs:
        func = func_pat.search(df).group(1)
        if func not in set(pair[1]):
            pair[0].append(df)
            pair[1].append(func)
    return pair

def minimise(defined_funs, partition_size):
    from more_itertools import chunked

    partition_size = partition_size or len(defined_funs[0])

    min_core = []
    for p in chunked(zip(*defined_funs), partition_size):
        pl = PowerLattice(zip(*p))
        sub_min_core = pl.minimise()
        print(f"sub before: {len(p)}, after: {len(sub_min_core)}")
        min_core.extend(sub_min_core)
        del pl
    return min_core

def run_minimisation(in_file, out_file, partition_size=None, ret=False):
    defined_funs = get_defined_funs(in_file)
    unique_funs = get_unique_funs(defined_funs)

    print('start size: ', len(unique_funs[0]))
    min_core = minimise(unique_funs, partition_size)
    print('final size: ', len(min_core))

    with open(out_file, 'w') as f:
        f.write("\n".join(min_core))

    if ret:
        return (len(unique_funs[0]), len(min_core))


if __name__ == '__main__':
    import sys

    defaults = ["Sygus/s38417.sl", "tmp", None]
    args = sys.argv[1:] + defaults[len(sys.argv)-1:]  # pad with defaults

    match sys.argv:
        case [_]: 
            file, out_file, partition_size = defaults
        case [_, f]: 
            file, out_file, partition_size = f, *defaults[1:]
        case [_, f, o]: 
            file, out_file, partition_size = f, o, defaults[2]
        case [_, f, o, p]: 
            file, out_file, partition_size = f, o, p
        case _: 
            raise Exception("Too many arguments")

    run_minimisation(file, out_file, partition_size)
