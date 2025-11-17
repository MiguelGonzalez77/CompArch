.ORIG x3000

; BR - Unconditional branch
BR LABEL_BR

; BRnzp - Branch if any condition is true
BRnzp LABEL_BRnzp

; BRp - Branch if positive
BRp LABEL_BRp

; BRn - Branch if negative
BRn LABEL_BRn

; BRz - Branch if zero
BRz LABEL_BRz

; BRnz - Branch if not zero
BRnz LABEL_BRnz

; BRnp - Branch if not positive
BRnp LABEL_BRnp

; BRzp - Branch if not negative
BRzp LABEL_BRzp

; Branch target labels
LABEL_BR   .FILL x3002
    ; Your code here
    ; ...

LABEL_BRnzp .FILL x3004
    ; Your code here
    ; ...

LABEL_BRp .FILL x3006   ; BRp
    ; Your code here
    ; ...

LABEL_BRn .FILL x3008   ; BRn
    ; Your code here
    ; ...

LABEL_BRz .FILL x300A   ; BRz
    ; Your code here
    ; ...

LABEL_BRnz  .FILL x300C  ; BRnz
    ; Your code here
    ; ...

LABEL_BRnp .FILL x300E  ; BRnp
    ; Your code here
    ; ...

LABEL_BRzp .FILL x3010  ; BRzp
    ; Your code here
    ; ...

.END
