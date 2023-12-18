# Nox
A programming language to test out stuff.

## Development

### Status
* The JIT is working for both Aarch64 and X86-64 for simple expressions, but am experimenting with the code generation for more complex control flow before continuing further with it. 
* Currently, there is one failing test (DeferredNestedFunDecl). This is due to that we do 2 separate passes and handle the global and local scopes differently, in which we lose some information.

### Debugging the JIT
Simplest way is to use the marco `OUTPUT_JIT`. Then inspect the content by the following command:
```bash
# Aarch64
llvm-objcopy -I binary -O elf64-aarch64 --rename-section=.data=.text,code jit.bin jit.elf && objdump --disassemble-all jit.elf
# X86-64
llvm-objcopy -I binary -O elf64-x86-64 --rename-section=.data=.text,code jit.bin jit.elf && objdump --disassemble-all jit.elf
```


### Example
```bash
./nox run examples/while-loop.nox --verbose
```

```
[DEBUG] (Main) ./nox/src/lib.c:61: Source of 'examples/while-loop.nox':
"""
fun test() int {
    a := 10
    while a < 20 {
        a = a + 1
    }
    return a
}

x := test()
"""

[DEBUG] (Main) ./nox/src/lib.c:61: AST:
Module: id=0 @ examples/while-loop.nox:1:1
  ├─ decl0  FunDecl: id=17, repr='fun', name='test' @ examples/while-loop.nox:1:1
  │         └─ body   FunBody: id=2, parent='0', block_id='1' decls='1308689936' @ examples/while-loop.nox:1:16
  │                   ├─ stmt0  VarDecl: id=4, name='a' @ examples/while-loop.nox:2:5
  │                   │         └─ expr   Literal: id=3, repr='10', type='integer' @ examples/while-loop.nox:2:10
  │                   ├─ stmt1  While: id=14 @ examples/while-loop.nox:3:5
  │                   │         ├─ cond   Binary: id=6, repr='<', op='<' @ examples/while-loop.nox:3:13
  │                   │         │         ├─ left   Identifier: id=5, repr='a' @ examples/while-loop.nox:3:11
  │                   │         │         └─ right  Literal: id=7, repr='20', type='integer' @ examples/while-loop.nox:3:15
  │                   │         └─ then   Block: id=8, parent='1', block_id='2' @ examples/while-loop.nox:3:18
  │                   │                   └─ stmt0  Assign: id=13, name='a' @ examples/while-loop.nox:4:11
  │                   │                             └─ expr   Binary: id=11, repr='+', op='+' @ examples/while-loop.nox:4:15
  │                   │                                       ├─ left   Identifier: id=10, repr='a' @ examples/while-loop.nox:4:13
  │                   │                                       └─ right  Literal: id=12, repr='1', type='integer' @ examples/while-loop.nox:4:17
  │                   └─ stmt2  Return: id=16, repr='return' @ examples/while-loop.nox:6:5
  │                             └─ expr   Identifier: id=15, repr='a' @ examples/while-loop.nox:6:12
  └─ stmt0  VarDecl: id=20, name='x' @ examples/while-loop.nox:9:1
            └─ expr   FunCall: id=19, name='test' @ examples/while-loop.nox:9:6


[DEBUG] (Main) ./nox/src/lib.c:61: Bytecode:
[0000]:  Add    sp     1                       
[0001]:  Call   [0007]             ─┐          
[0002]:  Mov    r3     r2           │          
[0003]:  Store  0      r3           │          
[0004]:  Add    sp     -1           │          
[0005]:  Mov    r2     r2           │          
[0006]:  Exit                       │          
───────────────────[0007]───────────│──────────
[0007]:  Push   bp                 <┘          
[0008]:  Mov    bp     sp                      
[0009]:  Add    sp     0                       
[000a]:  Add    sp     0                       
[000b]:  Mov    r2     10                      
[000c]:  Store  0      r2                      
[000d]:  Load   r2     0                 <┐    
[000e]:  Mov    r3     20                 │    
[000f]:  Lt     r2     r3                 │    
[0010]:  JmpZ   [0016] r2             ─┐  │    
[0011]:  Load   r2     0               │  │    
[0012]:  Mov    r3     1               │  │    
[0013]:  Add    r2     r3              │  │    
[0014]:  Store  0      r2              │  │    
[0015]:  Jmp    [000d]                 │ ─┘    
[0016]:  Load   r2     0              <┘       
[0017]:  Mov    r2     r2                      
[0018]:  Jmp    [0019]                      ─┐ 
[0019]:  Mov    sp     bp                   <┘ 
[001a]:  Pop    bp                             
[001b]:  Ret                                   

[DEBUG] (Main) ./nox/src/lib.c:108 JIT: 
Failed to JIT compile

[DEBUG] (Main) ./nox/src/lib.c:108 Interpreter:
Result: 20
```
