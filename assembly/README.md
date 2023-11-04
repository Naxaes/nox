
NASM (Netwide Assembler) syntax
GNU Assembler (GAS) syntax


clang -nostdlib -nodefaultlibs -c -o darwin-x86_64.o --target=darwin-x86_64-apple-darwin darwin-x86_64.s && ld -e _start -arch x86_64 -o darwin-x86_64.out darwin-x86_64.o && ./darwin-x86_64.out
clang -nostdlib -nodefaultlibs -c -o darwin-aarch64.o --target=darwin-aarch64-apple-darwin darwin-aarch64.s && ld -e _start -arch arm64 -o darwin-aarch64.out darwin-aarch64.o && ./darwin-aarch64.out

