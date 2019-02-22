.data
offset_factor:
    .word 0

.word print
.word convert_to_decimal

.text

.global add_nums, convert_to_decimal, print

add_nums:
    mov r0, r6[2]
    add r0, r6[4]
    add r0, $offset_factor
    ret

convert_to_decimal:
    push r1
    push r2
    mov r0, r6[8]
    mov r1, r6[6]

    cmp r0, 0
    jmpeq $skip

loop:
    div r0, 10
    add r1, 2
    cmp r0, 0
    jmpne $loop

skip:
    mov r1[0], r0
    sub r1, 2

    mov r0, r6[8]
    
    cmp r0, 0
    jmpeq $end

digit_loop:
    mov r2, r0
    div r0, 10
    mul r0, 10
    sub r2, r0
    div r0, 10
    add r2, '0'
    mov r1[0], r2
    sub r1, 2
    cmp r0, 0
    jmpne $digit_loop

end:
    pop r2
    pop r1
    ret

print:
    push r1
    mov r0, r6[4]

    mov r1, r0[0]
    cmp r1, 0
    jmpeq $print_end

print_loop:
    mov *0xFFFE, r1
    add r0, 2
    mov r1, r0[0]
    cmp r1, 0
    jmpne $print_loop 

print_end:
    pop r1
    ret
