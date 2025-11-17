
.ORIG x3000
LDW R0, R1, #offset ; Load word from memory at address R1 + offset into R0
ADD R2, R1, R3     ; Calculate the effective address by adding R1 and R3
LDW R0, R2, #0     ; Load word from memory at address R2 into R0
LDW R0, R1, R2     ; Load word from memory at address R1 + R2 into R0
offset .FILL #1
HALT
.END
```

