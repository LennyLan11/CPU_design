MOV R0, 0
CMP R0, 0
JE factorial_base_case
PUSH R0
DEC R0
CALL factorial
POP R1
MUL R0, R0, R1
JMP factorial_end
factorial_base_case:
MOV R0, 1
factorial_end: