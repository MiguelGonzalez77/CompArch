.ORIG x3000
LEA R0, TWOsCOMPNum1    ; Load the first number into R2
LDW R2, R2, #0  
LDW R3, R3, #0 
; now load it for LDB because addressability is 8-bit each memory location
LDB R0, R0, #0 ; Load the first byte of the 16-bit into R0
LDB R1, R3, #0 ; Load the second byte 
BRn NEGATIVE
BRp POSITIVE
BRz ZERO

POSITIVE  LEA R7, TWOsCOMPNum2 ; If the first number is positive
; then load the second number into R7
     ; Load the second number into R3
LDB R7, R7, #0; addressability is 8-bit each memory location
; storing the contents of R7 into R7
BRp BOTHNUMPOS ; check if second num is also pos
BRnz NORMAL ; if one is pos and other is neg
; then no overflow can occur

NEGATIVE LEA R7, TWOsCOMPNum2   ; If the first number is negative    ; Load the second number into R7
LDB R7, R0, #0
BRn BOTHNUMNEG ; check if second num is also neg
BRzp NORMAL ; go to the same branch as NORMAL case

ZERO LEA R7, TWOsCOMPNum2 
    ; If the first number is zero
   ; Load the second number into R7
LDB R7, R7, #0 ; addressability is 8-bit each memory location
BR NORMAL ; because first number is zero, no overflow can occur

NORMAL LEA R6, RESULT ; Load the address of the result into R6
LDB R6, R6, #0
STB R5, R6, #0 ; the addition is stored in R5 aka x3102
LEA R6, NO
LDB R6, R6, #0
LEA R4, OVERFLOW
LDB R4, R4, #0 ; load overflow result into R4
STB R6, R4, #0 ; store the result of overflow into R4
BRnzp DONE ; go to the end of the program

OVERFLOWADD LEA R6, RESULT 
LDB R6, R6, #0
STB R5, R6, #0 ; the addition is stored in R5 aka x3102
LEA R6, YES
LDB R6, R6, #0
LEA R4, OVERFLOW
LDB R4, R4, #0 ; load overflow result into R4
STB R6, R4, #0 ; store the result of overflow into R4
BRnzp DONE ; go to the end of the program

BOTHNUMPOS ADD R5, R7, R0
BRn OVERFLOWADD
BRzp NORMAL

BOTHNUMNEG ADD R5, R7, R0
BRp OVERFLOWADD
BRnz NORMAL

DONE HALT
TWOsCOMPNum1   .FILL x3100 ; The first number
TWOsCOMPNum2   .FILL x3101 ; The second number
RESULT .FILL x3102 ; store the result in 0x3102
OVERFLOW .FILL x3103 ; Overflow = 1 in 0x3103 and Overflow = 0 in 0x3103
YES .FILL #1
NO .FILL #0
.END

