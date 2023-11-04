.global _start
.align 2

stdout = 1
exit = 1
write = 4

_start:
    /* syscall write(int fd, const void* buf, size_t count) */
    mov    X0, #stdout          // 1 = stdout
    adr    X1, message          // string to print
    mov    X2, message_len      // length of our string
    mov    X16, #write          // Unix write system call
    svc    #0x80                // Call kernel to output the string

    /* syscall exit(int status) */
    mov     X0, #0              // Use 0 return code
    mov     X16, #exit          // System call number 1 terminates this program
    svc     #0x80               // Call kernel to terminate the program

message:
    .ascii  "Hello, arm64!\n"
    message_len = . - message
