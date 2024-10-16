(set-logic BV)

(synth-fun SC ((s (_ BitVec 4)) (t (_ BitVec 4))) Bool
    ((Start Bool) (StartBv (_ BitVec 4)))
    (
    (Start Bool (true false (not Start) (and Start Start) (or Start Start) (= StartBv StartBv) (bvult StartBv StartBv) (bvslt StartBv StartBv) (bvuge StartBv StartBv) (bvsge StartBv StartBv)))
    (StartBv (_ BitVec 4) (s t #x0 #x8 #x7 (bvneg StartBv) (bvnot StartBv) (bvadd StartBv StartBv) (bvsub StartBv StartBv) (bvand StartBv StartBv) (bvlshr StartBv StartBv) (bvor StartBv StartBv) (bvshl StartBv StartBv)))))

(declare-var s (_ BitVec 4))
(declare-var t (_ BitVec 4))
(define-fun udivtotal ((a (_ BitVec 4)) (b (_ BitVec 4))) (_ BitVec 4)
    (ite (= b #x0) #xF (bvudiv a b)))
(define-fun uremtotal ((a (_ BitVec 4)) (b (_ BitVec 4))) (_ BitVec 4)
    (ite (= b #x0) a (bvurem a b)))
(define-fun case ((x (_ BitVec 4)) (s (_ BitVec 4)) (t (_ BitVec 4))) Bool
    (bvsge (bvand x s) t))
(constraint (= (or (case #x0 s t) (case #x1 s t) (case #x2 s t) (case #x3 s t) (case #x4 s t) (case #x5 s t) (case #x6 s t) (case #x7 s t) (case #x8 s t) (case #x9 s t) (case #xA s t) (case #xB s t) (case #xC s t) (case #xD s t) (case #xE s t) (case #xF s t)) (SC s t)))

(check-synth)