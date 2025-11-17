.ORIG x1200
; AND R6, R6, #0
; don't clear this stack pointer
ADD R6, R6 #-2
; STW R1, R6, #0
STW R0, R6, #0
; LEA R0, INC
ADD R6, R6, #-2
STW R1, R6, #0
LEA R0, COUNTER
LDW R0, R0, #0
LDW R1, R0, #0
ADD R1, R1, #1
STW R1, R0, #0
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
LDW R1, R6, #0
ADD R6, R6, #2
LDW R0, R6, #0
ADD R6, R6, #2
RTI
; HALT 
COUNTER .FILL x4000
.END