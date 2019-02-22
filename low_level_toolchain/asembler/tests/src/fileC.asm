.global firstNum
.global secondNum
.global add_nums
.global START, kbd_routine, convert_to_decimal, print

.rodata
kbd_input_reg:
    .word 0xFFFC

msg_greet:
    .word 'Z', 'd', 'r', 'a', 'v', 'o', '!', 0x10, 0

msg_text_input:
    .word 'U', 'n', 'e', 's', 'i', 't', 'e', ' ', 't', 'e', 'k', 's', 't', ':', 0

msg_input:
    .word 'U', 'n', 'e', 's', 'i', 't', 'e', ' ', 'b', 'r', 'o', 'j', ':', 0

msg_bigger:
    .word ' ', 'j', 'e', ' ', 'v', 'e', 'c', 'i', 0x10, 0

msg_equal:
    .word 'J', 'e', 'd', 'n', 'a', 'k', 'i', ' ', 's', 'u', 0x10, 0

lprint:
	.word print

.data
char_buffer: .word 0

ldprint:
	.word print

sem:  
    .char 0x0

output_buffer:
    .skip 10

.bss
    .skip 512
stack:

.text
kbd_routine:
    push r0
    push r1
    mov r0, kbd_input_reg
    mov r1, r0[0]

    mov char_buffer, r1

    mov r0, 1
    mov sem, r0

    mov r1, r6[6]
    and r1, 0x7FFF
    mov r6[6], r1

    pop r1
    pop r0
    iret


talk_back:

    mov r0, 0
    mov sem, r0    
    mov r0, PSW
    or r0, 0x8000
    mov PSW, r0

idle:
    mov r0, sem
    test r0, 1
    jmpeq $idle

    mov r0, char_buffer
    cmp r0, 27
    jmpeq $talk_back_end

    mov *0xFFFE, r0
    mov r0, 0
    mov sem, r0
    jmp $talk_back

talk_back_end:
    mov r0, 0x10
    mov *0xFFFE, r0

    ret

fetch_num:
    push r1
    mov r0, 0
    mov sem, r0

fetch_start:
    mov r1, PSW
    or r1, 0x8000
    mov PSW, r1

fetch_idle:
    mov r1, sem
    cmp r1, 0
    jmpeq $fetch_idle

    mov r1, char_buffer
    cmp r1, 10
    jmpeq $fetch_end

    mov *0xFFFE, r1
    sub r1, '0'
    mul r0, 10
    add r0, r1
    mov r1, 0
    mov sem, r1
    jmp $fetch_start

fetch_end:

    mov r1, 0x10
    mov *0xFFFE, r1
    pop r1
    ret

START:
    mov r1, &kbd_routine
    mov *6, r1

    mov r6, &stack
    push firstNum
    push secondNum
    call &add_nums
    add r6, 0x4

    push r0
    push &output_buffer
    call &convert_to_decimal
    add r6, 0x4
    
    push &output_buffer
    call &print
    add r6, 0x2

    mov r5, 0x10
    mov *0xFFFE, r5

    push &msg_greet
    call &print
    add r6, 0x2

    push &msg_text_input
    call &print
    add r6, 0x2

    call &talk_back 

    mov r0, PSW
    or r0, 0x8000
    or r0, 0x2000
    mov PSW, r0
    
    .global time_counter

time_loop:
    mov r0, time_counter
    cmp r0, 10
    jmpne $time_loop

    mov r0, PSW
    and r0, 0xDFFF
    mov PSW, r0


    push &msg_input
    call &print
    add r6, 0x2

    call &fetch_num

    push r0

    push r0
    push &output_buffer
    call &convert_to_decimal
    add r6, 0x4

    push &output_buffer
    call &print
    add r6, 0x2

    mov r2, 0x10
    mov *0xFFFE, r2

    push &msg_input
    call &print
    add r6, 0x2

    call &fetch_num
    push r0

    push r0
    push &output_buffer
    call &convert_to_decimal
    add r6, 0x4

    push &output_buffer
    call &print
    add r6, 0x2

    mov *0xFFFE, r2

    pop r1
    pop r3

    mov r0, r3
    cmp r3, r1
    jmpgt $print_bigger

    cmp r3, r1
    movne r0, r1
    jmpne $print_bigger

    push &msg_equal
    callal &print
    add r6, 0x2
    jmp $finish

print_bigger:
    push r0
    push &output_buffer
    call &convert_to_decimal
    add r6, 0x4

    push &output_buffer
    call &print
    add r6, 0x2

    push &msg_bigger
    call &print
    add r6, 0x2

finish:
    cmp 0, r1
    jmpgt $prog_end
    cmp r1, 8
    jmpgt $prog_end

    mov r0, r3
    shl r0, r1
    
    push r0
    push &output_buffer
    call &convert_to_decimal
    add r6, 0x4

    push &output_buffer
    call &print
    add r6, 0x2

    mov *0xFFFE, r2

    mov r0, r3
    shr r0, r1
    
    push r0
    push &output_buffer
    call &convert_to_decimal
    add r6, 0x4

    push &output_buffer
    call &print
    add r6, 0x2

    mov *0xFFFE, r2
prog_end:

    mov r0, r3
    mul r0, r1

    push r0
    push &output_buffer
    call &convert_to_decimal
    add r6, 0x4

    push &output_buffer
    call &print
    add r6, 0x2

    mov *0xFFFE, r2
    
    mov r0, r3
    div r0, r1

    push r0
    push &output_buffer
    call &convert_to_decimal
    add r6, 0x4

    push &output_buffer
    call &print
    add r6, 0x2

    mov *0xFFFE, r2

    call *14       
