.ORIG x3000 # origin of starting address
AND R0, R0, #0 # set R0 to 0
LEA R1, ADDR # Load jump based on ADDR which will be the address of the instruction ADD R0, R0, #2
JSRR R1
TRAP x25
ADDR ADD R0, R0, #2
RET
.END
