
//args    RDI,RSI,RDX,RCX,R8,R9
//syscall RDI,RSI,RDX,R10,R8,R9
//restore RBX, RBP, and R12–R15

	.global Linux_Syscall_Clone
	.global Linux_ThreadProc
	.text


//enters with (args,call,stack_size,stack ptr) on the stack
Linux_ThreadProc:
	popq %rdi
	popq %rax

	callq *%rax

	movq %rax,%rcx // save the return value

	popq %rsi
	popq %rdi
	movl $11,%eax
	syscall //sys_munmap

	movq %rcx,%rdi
	movl $60,%eax
	syscall //sys_exit

// u32 clone_flags,void* newsp,void* parent_tid,void* child_tid
Linux_Syscall_Clone:
	movq %rcx,%r10
	movl $56,%eax
	syscall

	ret