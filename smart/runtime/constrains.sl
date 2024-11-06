; dataa_copy datab_copy add_sub result

; (constraint (=(inv false true #b00000000 #b10101000 #b00010001 #b11111001 #b00100000 ) false))

(constraint (= (inv #b00000000 #b00000000 false #b00000001 ) false))
(constraint (= (inv #b00000000 #b00000000 true #b00000001 ) false))