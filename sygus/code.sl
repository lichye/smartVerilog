(set-logic BV)


(synth-fun i1 ((ah (_ BitVec 8)) (i2 (_ BitVec 8))) (_ BitVec 8)
    ((Start (_ BitVec 8)))
    (
        (Start (_ BitVec 8) 
            (ah i2
            (ite (= Start (_ bv0 8)) Start Start) 
            (bvneg Start) 
            (bvnot Start)
            (bvxor Start Start) 
            (bvadd Start Start) 
            (bvsub Start Start) 
            (bvand Start Start) 
            (bvlshr Start Start) 
            (bvor Start Start) 
            (bvshl Start Start)
            )
        )
    )
)



(constraint (= (i1 #b00000000 #b00011100) #b11111100))
(constraint (= (i1 #b00000001 #b00000010) #b00000010))
(constraint (= (i1 #b00000011 #b00000010) #b00000010))
(constraint (= (i1 #b11100000 #b10100000) #b10100000))
(constraint (= (i1 #b11111100 #b10100011) #b10100011))

(check-synth)
