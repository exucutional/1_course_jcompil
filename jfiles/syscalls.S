section .text
global _start
_start:
_SYSCALLS:
cmp rax, 0
je _SYSEXIT
cmp rax, 1
je _SYSWRITE
cmp rax, 2
je _SYSREAD
jmp _SYSNOP
_SYSEXIT:
mov rax, 60
xor rdi, rdi
syscall
ret
_SYSWRITE:
mov rax, [rsp + 8]
mov rcx, 1
mov rbx, 10
sub rsp, 16
_SYSWRITEAG:
xor rdx, rdx
div rbx
add rdx, '0'
mov byte [rsp + rcx - 1], dl 
cmp rax, 0
je _SYSWRITEEX
inc rcx
jmp _SYSWRITEAG
_SYSWRITEEX:
mov rdi, 1
mov rdx, 1
mov rax, 1
lea rsi, [rsp + rcx]
_SYSWRITEEXAG:
sub rsi, 1
push rcx
syscall
pop rcx
loop _SYSWRITEEXAG
add rsp, 16
ret
_SYSREAD:
mov rax, 0
sub rsp, 16
mov rdi, 0
mov rsi, rsp
mov rdx, 16
syscall
xor rdi, rdi
mov rbx, 10
xor rax, rax
_SYSREADAG:
xor rcx, rcx
mov cl, byte [rsp + rdi]
cmp cl, 0xa
je _SYSREADEX
sub rcx, '0'
mul rbx
add rax, rcx
inc rdi
jmp _SYSREADAG
_SYSREADEX:
add rsp, 16
_SYSNOP:
ret
