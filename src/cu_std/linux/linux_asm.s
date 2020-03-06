
//args    RDI,RSI,RDX,RCX,R8,R9
//syscall RDI,RSI,RDX,R10,R8,R9
//restore RBX, RBP, and R12–R15

	.global Linux_Syscall_Clone
	.global Linux_ThreadProc
	.text


//enters with (TCreateThreadStruct*) on the stack
Linux_ThreadProc:

	pushq %rbp
	movq %rsp,%rbp


	movq 0x8(%rbp),%rdi
	movq 0x10(%rbp),%rax

	callq *%rax

// save the return value
	movq %rax,%rcx 


//sys_munmap
	movq 0x18(%rbp),%rdi
	movq 0x20(%rbp),%rsi
	movl $11,%eax
	syscall 

	popq %rbp
	popq %rdi

//sys_exit
	movq %rcx,%rdi
	movl $60,%eax
	syscall 

// u32 clone_flags,void* newsp,void* parent_tid,void* child_tid
Linux_Syscall_Clone:
	movq %rcx,%r10
	movl $56,%eax
	syscall

	ret

