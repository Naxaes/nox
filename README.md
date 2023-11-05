# Nox
A programming language.

More information to come...

## Development

### Debugging the JIT
Simplest way is to use the marco `OUTPUT_JIT`. Then inspect the content by the following command:
```bash
# Aarch64
llvm-objcopy -I binary -O elf64-aarch64 --rename-section=.data=.text,code jit.bin jit.elf && objdump --disassemble-all jit.elf
# X86-64
llvm-objcopy -I binary -O elf64-x86-64 --rename-section=.data=.text,code jit.bin jit.elf && objdump --disassemble-all jit.elf
```
