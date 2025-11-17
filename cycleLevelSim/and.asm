.ORIG x3000

AND R0, R0, #0 ; Clear R0
AND R1, R1, #0 ; Clear R1
; Set values in R0 and R1
ADD R0, R0, #5 ; Set R0 to 5
ADD R1, R1, #3 ; Set R1 to 3
AND R2, R0, R1 ; Perform AND operation on R0 and R1, store result in R2
; result should be 1
HALT ; Halt the program
.END
