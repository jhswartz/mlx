.global run
run:
	push %rbp
	movq %rsp, %rbp
	call *%rdi
	leave
	ret
