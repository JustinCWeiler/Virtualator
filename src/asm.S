.intel_syntax noprefix

.globl _memcpy
_memcpy:
	movsb
	dec rdx
	jnz _memcpy
	ret

.globl _add
_add:
	add [rdi], rsi
	ret

.globl push_pop_test
push_pop_test:
	mov rax, rsp

	mov rsp, rdi
	push rsi
	pop rsi

	mov rsp, rax
	ret
