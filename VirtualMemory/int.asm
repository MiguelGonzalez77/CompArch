.ORIG x1200
; AND R6, R6, #0
; don't clear this stack pointer
ADD R6, R6 #-2
; STW R1, R6, #0
STW R0, R6, #0
; LEA R0, INC
ADD R6, R6, #-2
STW R1, R6, #0
ADD R6, R6, #-2 ; added for the second push for pte
STW R3, R6, #0 
ADD R6, R6, #-2 
STW R4, R6, #0

LEA R0, TABLE
LDW R0, R0, #0
LEA R1, PTE
LDW R1, R0, #0
LEA R3, BITONE
LDW R3, R3, #0

LOOP LDW R4, R0, #0
AND R4, R4, R3
STW R4, R0, #0
ADD R0, R0, #2 ; inc to next entry
ADD R1, R1, #-1 ; counter
BRP LOOP
BRNZ FIN




FIN LDW R4, R6, #0
ADD R6, R6, #2
LDW R3, R6, #0
ADD R6, R6, #2
LDW R1, R6, #0
ADD R6, R6, #2
LDW R0, R6, #0
ADD R6, R6, #2
;ADD R1, R1, #1
;STW R1, R0, #0
; ADD R6, R6, #-1 
; STR R0, R6, #0 
; LDR R1, R6, #0 
; ADD R6, R6, #1 
;LDW R6, STACK
;LOOP_PUSH
    ;ADD R6, R6, #-1
    ;STW R6, R0, #0
    ;BRnzp LOOP_PUSH
;LOOP_POP
    
    ;LDW R0, R6, #0
    ;ADD R6, R6, #1
    ;BRnzp LOOP_POP

;STACK .BLKW x4000  ; memory
; LDW R1, R6, #0
; ADD R6, R6, #2
; LDW R0, R6, #0
; ADD R6, R6, #2
RTI
; HALT 
TABLE .FILL x1000
PTE .FILL x80
BITONE .FILL xFFFE
.END