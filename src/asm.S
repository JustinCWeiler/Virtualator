.intel_syntax noprefix

.globl _memcpy
_memcpy:
label:
	movsb
	dec rdx
	jnz label
	ret
