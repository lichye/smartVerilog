(set-logic BV)


(synth-fun inv ((ah (_ BitVec 8)) (i1 (_ BitVec 8))(i2 (_ BitVec 8))) Bool
    ((Start Bool) (StartBv (_ BitVec 8)))
    (   (Start Bool 
            (
                ; true false 
                (not Start) 
                (and Start Start) 
                (or Start Start)
                (bvult StartBv StartBv) 
                (bvslt StartBv StartBv) 
                (bvuge StartBv StartBv) 
                (bvsge StartBv StartBv)
                (not (= StartBv (_ bv0 8)))
            )
        )
        (StartBv (_ BitVec 8) 
            (
                ah i1 i2
                ; (ite (= StartBv (_ bv0 8)) StartBv StartBv) 
                (bvneg StartBv) 
                (bvnot StartBv)
                (bvand StartBv StartBv)
                (bvxor StartBv StartBv) 
                (bvadd StartBv StartBv) 
                (bvsub StartBv StartBv) 
                 
                (bvlshr StartBv StartBv) 
                (bvor StartBv StartBv) 
                (bvshl StartBv StartBv)
            )
        )
    )
)



(constraint (= (inv #b00000000 #b00011100 #b11111100) true))
(constraint (= (inv #b00000001 #b00000010 #b00000010) true))
(constraint (= (inv #b00000011 #b00000010 #b00000010) true))
(constraint (= (inv #b11100000 #b10100000 #b10100000) true))
(constraint (= (inv #b11111100 #b10100011 #b10100011) true))

(check-synth)
