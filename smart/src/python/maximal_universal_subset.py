# Taken from Mint -> https://alexeyignatiev.github.io/software/mintt ijk/
#==============================================================================
from pysmt.smtlib.parser import SmtLibParser
from pysmt.shortcuts import Not, ForAll, is_sat, is_unsat, get_model, qelim, Bool, Not, Solver
from pysmt.exceptions import SolverReturnedUnknownResultError

import re
import z3

from io import StringIO
#==============================================================================


class Mistral:
    """
        Mistral solver class.
    """

    def __init__(self, simplify=True, incremental=True):
        """
            Constructor.
        """

        self.simplify = simplify
        self.incremental = incremental
        self.cost = 0
        self.sname = 'z3'

    def solve(self, query):
        """
            This method implements find_msa() procedure from Fig. 2
            of the dillig-cav12 paper.
        """

        self.script = SmtLibParser().get_script(StringIO(query))
        self.formula = self.script.get_last_formula()
        self.formula = self.formula.simplify()

        self.fvars = self.formula.get_free_variables()

        if get_model(self.formula, solver_name=self.sname) == None:
            return None

        mus = self.compute_mus(frozenset([]), self.fvars, 0)
        # return MSA (minimal satisfying assignment)
        return self.fvars- mus

    def compute_mus(self, X, fvars, lb):
        """
            Algorithm implements find_mus() procedure from Fig. 1
            of the dillig-cav12 paper.
        """

        if not fvars or len(fvars) <= lb:
            return frozenset()

        best = set()
        x = frozenset([next(iter(fvars))]) 

        if self.get_model_forall(X.union(x)):
            Y = self.compute_mus(X.union(x), fvars - x, lb - 1)

            cost_curr = len(Y) + 1
            if cost_curr > lb:
                best = Y.union(x)
                lb = cost_curr

        Y = self.compute_mus(X, fvars - x, lb)
        if len(Y) > lb:
            best = Y

        return best

    def get_model_forall(self, x_univl):
        return get_model(ForAll(x_univl, self.formula),
                solver_name=self.sname)



# Regex definitions
def_pat = re.compile(r'.*define-fun.*')
func_pat = re.compile(r'(?:Bool|\(_\s+BitVec\s+\d+\))\s+(.*)\)')
var_pat = re.compile(r'\((\w+)\s+(?:(Bool)|\(_\s+(BitVec)\s+(\d+)\))\)')

def get_defined_funs(file):
    with open(file, 'r') as f:
        content = f.read()
        matches = def_pat.findall(content)
    return set(matches)

def generate_vars(expr, d):
    for m in var_pat.findall(expr):
        match m:
            case [n, '', _, w]:
                tmp = z3.BitVec(n, int(w))
            case [n, _, '', '']:
                tmp = z3.Bool(n)

        d[n] = tmp

def gen_vars_get_fun(expr, d):
    generate_vars(expr, d)
    return func_pat.search(expr).group(1)

def get_mus(exprs, d, m):
    assertion = f"""
    (assert
        (and {(' ').join(exprs)})
    )
    """
    z3_assertion = z3.parse_smt2_string(assertion, decls=d)
    s = z3.Solver()
    s.add(z3_assertion)

    minimal_uni_subset = m.solve(s.to_smt2())
    return minimal_uni_subset

def get_variables(file):
    with open(file, 'r') as f:
        variables = set(v for v in f.read().split())
    return variables

def run_get_mus(file = 'ibex_controller'):
    variables = get_variables(f"runtime/variables.txt")
    matches = get_defined_funs(f"runtime/SygusResult.sl")
    m = Mistral()

    d = {}
    assertions = {gen_vars_get_fun(a, d) for a in matches}
    mas = {str(a) for a in get_mus(assertions, d, m)}
    underspecified = variables - mas
    return underspecified