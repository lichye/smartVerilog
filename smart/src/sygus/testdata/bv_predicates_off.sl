(set-logic BV)
( synth-fun inv ((valid Bool) (count (_ BitVec 8)) (limit (_ BitVec 8)) ) Bool
(
(Expr Bool) (Atom Bool)
(MixBv8  (_ BitVec 8))
)
(
(Expr Bool
	(
	 Atom
	 (=> Atom Atom)
	)
)
(Atom Bool
	(
	(= Atom Atom)
	(not Atom)
	(and Atom Atom)
	(or Atom Atom)
	valid
	)
)
(MixBv8 (_ BitVec 8)
    (
	count
	limit
	(bvneg MixBv8)
	(bvnot MixBv8)
	(bvand MixBv8 MixBv8)
	(bvor MixBv8 MixBv8)
	(bvadd MixBv8 MixBv8)
	(bvsub MixBv8 MixBv8)
	(bvmul MixBv8 MixBv8)
	)
)
)
)
; False Constraints below
(check-synth)
