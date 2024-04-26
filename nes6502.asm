DEFAULT REL
bits    64

section .text
    global _reset6502z

_zerocalc:
    ret

_signcalc:
    ret

_carrycalc:
    ret

_overflowcalc:
    ret

_push16:
    ret

_pull16:
    ret

_push8:
    ret

_pull8:
    ret

_reset6502z:
    push rax
    xor eax,eax
    mov [r_pc], ax
    mov [r_sp], al
    mov [r_a], al
    mov [r_x], al
    mov [r_y], al
    mov [r_status], al
    mov [instructions], eax
    mov [clockticks], eax
    mov [clockgoal], eax
    mov [oldpc], ax
    mov [ea], ax
    mov [reladdr], ax
    mov [value], ax
    mov [result], ax
    mov [opcode], al
    mov [oldstatus], al
    mov [penaltyop], al
    mov [penaltyaddr], al
    pop rax
    ret

_adc:
    ret

section .data
r_a            DB  0xFF
r_x            DB  0xFF
r_y            DB  0xFF
r_status       DB  0xFF
r_sp           DB  0xFF
r_pc           DW  0xFF,0xFF
instructions   DD  0xFF,0xFF,0xFF,0xFF
clockticks     DD  0xFF,0xFF,0xFF,0xFF
clockgoal      DD  0xFF,0xFF,0xFF,0xFF
oldpc          DW  0xFF,0xFF
ea             DW  0xFF,0xFF
reladdr        DW  0xFF,0xFF
value          DW  0xFF,0xFF
result         DW  0xFF,0xFF
opcode         DB  0xFF
oldstatus      DB  0xFF
penaltyop      DB  0xFF
penaltyaddr    DB  0xFF

_read6502      DD  0xFF,0xFF,0xFF,0xFF
_write6502     DD  0xFF,0xFF,0xFF,0xFF
