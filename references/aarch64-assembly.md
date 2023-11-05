# AArch64 Assembly

## Documentation
Download: https://developer.arm.com/documentation/ddi0487/latest/arm-architecture-reference-manual-armv8-for-armv8-a-architecture-profile
Website: https://developer.arm.com/documentation

Interesting chapters:
- Charpter B1 - The AArch64 Application Level Programmers’ Model
- C1.2 Structure of the A64 assembler language
- Chapter C4 - A64 Instruction Set Encoding


## Utilities
Compile a C file to assembly
```bash
clang -o a.s -S -arch arm64 a.c
```
Compile an assembly to an object file
```bash
as -o a.o -arch arm64 a.s
```
View an object file
```bash
# View a MacOS Macho object file
objdump --disassemble-all --macho a.o
````
Create executable (use -e _start if you've written your own _start label in assembly) on MacOS
```bash
clang -o a.s -S -arch arm64 a.c
as -o a.o -arch arm64 a.s
ld -o a.out a.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` -arch arm64
```
Inspect raw binary
```bash
llvm-objcopy -I binary -O elf64-littleaarch64 --rename-section=.data=.text,code raw.bin raw.elf
objdump --disassemble-all raw.elf
````

## General info
* Supports only a single instruction set, A64.
* Fixed-length instructions, 32 bits.
* Byte addressable
* Two operating modes:
    * AARCH64: 64-bit instructions
    * AARCH32: 32-bit instructions
    * Both have user and supervisor modes
* A64 instructions have a fixed length of 32 bits and are always little-endian.

### Assembly format
* \<operator> \<destination> \<source>
* \<operator> \<destination> #\<immediate>
* \<operator> \<destination>, [\<address>]

* For example:
```AARCH64
.global _start
_start:
    mov x0, #0x4     // Move 0x4 into register x0
    ldr x1, =msg     // Load the address of msg into register x1
    ldr x2, [x1]     // Load "H", the value at the address in register x1 into register x2
    ldr x3, [x1, #6] // Load "W", the value at the address in register x1, plus 6 bytes, into register x3
.data
    msg: .ascii "Hello World!\n"
```

### Instruction set
* Is always 32-bit.

* svc: Supervisor call
* mov: Move
* ldr: Load register
* str: Store register
* add: Add
* sub: Subtract
* mul: Multiply
* div: Divide
* ldr: Load register
* str: Store register
* stp: Store pair
* ldp: Load pair
* stur: Store unscaled register
* ldur: Load unscaled register
* bl: Branch with link


### Calling convention
* x31 (SP): Stack pointer or a zero register, depending on context.
* x30 (LR): Procedure link register, used to return from subroutines.
* x29 (FP): Frame pointer.
* x19 to x28: Callee-saved.
* x18 (PR): Platform register. Used for some operating-system-specific special purpose, or an additional caller-saved register.
* x16 (IP0) and x17 (IP1): Intra-Procedure-call scratch registers.
* x9 to x15: Local variables, caller saved.
* x8 (XR): Indirect return value address.
* x0 to x7: Argument values passed to and results returned from a subroutine.


### Encoding
```
| 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16   15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 |
|--|     |-----------|
op0           op1

| op0  |  op1   |  Group
------------------------------------------------------------------------------------------------
|  0   |  0000  |  Reserved
|  1   |  0000  |  SME encodings
|  -   |  0001  |  Unallocated
|  -   |  0010  |  SVE encodings
|  -   |  0011  |  Unallocated
|  -   |  100x  |  Data processing -- Immediate
|  -   |  101x  |  Branches, Exception Generating and System instructions
|  -   |  x1x0  |  Loads and Stores
|  -   |  x101  |  Data processing -- Register
|  -   |  x111  |  Data processing -- Scalar Floating-Point and Advanced SIMD


Immediate
| 31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16   15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0 |
|----------||-----------|
100           op0
| op0  |  Group
---------------------------------------------------------------------------------------
| 00x  | PC-rel. addressing
| 010  | Add/subtract (immediate)
| 011  | Add/subtract (immediate, with tags)
| 100  | Logical (immediate)
| 101  | Move wide (immediate)
| 110  | Bitfield
| 111  | Extract


Add
| 31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16   15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0 |
sf  op  S   1   0   0   0   1   0   sh |-----------------------------------------------||------------------||------------------|
imm12                                       Rn                Rd
| sf | op | S  |  Group
---------------------------------------------------------------------------------------
| 0  | 0  | 0  | ADD (immediate) 32-bit variant
| 0  | 0  | 1  | ADDS (immediate) 32-bit variant
| 0  | 1  | 0  | SUB (immediate) 32-bit variant
| 0  | 1  | 1  | SUBS (immediate) 32-bit variant
| 1  | 0  | 0  | ADD (immediate) 64-bit variant
| 1  | 0  | 1  | ADDS (immediate) 64-bit variant
| 1  | 1  | 0  | SUB (immediate) 64-bit variant
| 1  | 1  | 1  | SUBS (immediate) 64-bit variant
```



### Registers
* R0-R30: 31 General purpose 64-bit registers
  Can be accessed as:
  * X0-X30 - 64-bit registers,where X30 is the Procedure call link register
  * W0-W30 - 32-bit registers
* V0-V31: 32 128-bit registers for Advanced SIMD and scalar floating-point operations
  Can be accessed as:
  * Q0-Q31 - 128-bit registers
  * D0-D31 - 64-bit registers
  * S0-S31 - 32-bit registers
  * H0-H31 - 16-bit registers
  * B0-B31 - 8-bit registers
* Z0-Z31: 32 scalable vector registers
  Can be accessed as:
  * Q0-Q31 - 128-bit registers
  * D0-D31 - 64-bit registers
  * S0-S31 - 32-bit registers
  * H0-H31 - 16-bit registers
  * B0-B31 - 8-bit registers
* SP: 64-bit dedicated Stack pointer
* PC: 64-bit Program counter (cannot be accessed directly, only through branch instructions)



## Hello world example
```AArch64
.global _start  // Provide program starting address to linker
.align 2		// Make sure everything is aligned properly

// Setup the parameters to print hello world
// and then call the Kernel to do it.
_start: mov	X0, #1		// 1 = StdOut
	adr	X1, helloworld 	// string to print
	mov	X2, #13	    	// length of our string
	mov	X16, #4		    // Unix write system call
	svc	#0x80		    // Call kernel to output the string

// Setup the parameters to exit the program
// and then call the kernel to do it.
	mov     X0, #0		// Use 0 return code
	mov     X16, #1		// System call number 1 terminates this program
	svc     #0x80		// Call kernel to terminate the program

helloworld:
    .ascii "Hello World!\n"
```
Compiling and running:
```bash
(cat << EOF
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

EOF
) >> hello_world.s
as -o hello_world.o -arch arm64 hello_world.s
ld -o hello_world.out -e _start -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` -arch arm64 hello_world.o
./hello_world.out
rm hello_world.s hello_world.o hello_world.out
```