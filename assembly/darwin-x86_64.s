.text

.global _start

stdout = 1
write = 0x2000004


_start:
    # write(stdout, $message, 14)
    movl $write, %eax
    movl $stdout, %ebx
    movq message@GOTPCREL(%rip), %rsi
    movq $14, %rdx
    syscall

    movl $0x2000001, %eax
    movl $0, %ebx
    syscall


.data

message:
    .ascii "Hello x86_64!\n\0"
    message_len = . - message

