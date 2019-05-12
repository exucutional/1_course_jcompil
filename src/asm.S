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
xor rcx, rcx
mov rbx, 10
sub rsp, 16
_SYSWRITEAG:
div rbx
add rdx, '0'
mov byte [rsp + rcx], rdx 
cmp rax, 0
je _SYSWRITEEX
inc rcx
jmp _SYSWRITEAG
_SYSWRITEEX:
mov rdx, rcx
mov rsi, rsp
mov rdi, 1
mov rax, 1
syscall
add rsp, 16
ret
_SYSREAD:
mov rbx, 10
sub rsp, 1
mov rdi, 0
mov rsi, rsp
mov rdx, 1
_SYSREADAG:
mov rcx, rax
mov rax, 0
syscall
mov rax, rcx
mov rcx, byte [rsp]
cmp rcx, 0xA
je _SYSREADEX:
add rcx, '0'
mul rbx
add rax, rcx
jmp _SYSREADAG
_SYSREADEX:
add rsp, 1
ret
