.org 0x8000

start:
    ; Simple loop that bumps R0 and then loops forever
    ADD R0, #1      ; R0 = R0 + 1
loop:
    ADD R0, #1      ; R0 = R0 + 1 again
    ; JZ done       ; (flags/conditional not wired yet in this ISA subset)
    JMP loop        ; infinite loop at label 'loop'
done:
    HALT
