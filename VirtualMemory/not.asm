.ORIG x3000

AND R2, R0, R0   ; Clear R2
AND R3, R0, R0   ; Clear R3

ADD R2, R1, R0   ; Move the value of R1 to R2
NOT R2, R2       ; NOT R2

ADD R3, R4, R0   ; Move the value of R4 to R3
NOT R3, R3       ; NOT R3

.END
