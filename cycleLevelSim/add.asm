.ORIG x3000
    ADD R0, R0, #0 ; Clear R0
    ADD R2, R1, #-1 ; Add -1 to R1 and store in R2
    ADD R2, R2, R2; Add R2 to R2
    HALT
.END
