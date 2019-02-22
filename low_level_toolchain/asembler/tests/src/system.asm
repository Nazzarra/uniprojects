.global add_nums, convert_to_decimal, print
.global time_counter

.data

.word greetings
.word timer
.word die
.word nop
.word nop
.word nop
.word nop
.word nop

time_counter:
    .word 1

buffer:
    .skip 20

.rodata
msg_start:
    .word 'P', 'r', 'o', 's', 'a', 'o', ' ', 'j', 'e', ' ', 0

msg_end:
    .word ' ', 's', 'e', 'k', 'u', 'n', 'd', 0

msg_illegal:
    .word 'I', 'L', 'L', 'E', 'G', 'A', 'L', ' ', 'I', 'N', 'S', 'T', 'R', 'U', 'C', 'T', 'I', 'O', 'N', 0x10, 0

.text
nop:
    iret

die:
    mov r0, 0

    mov r1, r0[msg_illegal]
loop:
    mov *0xFFFE, r1
    add r0, 1
    mov r1, r0[msg_illegal]
    cmp r1, 0
    jmpne $loop

    call *14

greetings:
    mov r0, ':'
    mov *0xFFFE, r0
    mov r0, ')'
    mov *0xFFFE, r0
    mov r0, 0x10
    mov *0xFFFE, r0
    mov r0, 'G'
    mov *0xFFFE, r0
    mov r0, 'o'
    mov *0xFFFE, r0
    mov r0, 0x10
    mov *0xFFFE, r0
    iret

timer:
    push r0
    
    push &msg_start
    call &print
    add r6, 0x2

    push time_counter
    push &buffer
    call &convert_to_decimal
    add r6, 0x4

    push &buffer
    call &print
    add r6, 0x2

    push &msg_end
    call &print
    add r6, 0x2

    mov r0, 0x10
    mov *0xFFFE, r0

    mov r0, time_counter
    add r0, 1
    mov time_counter, r0

    pop r0
    iret