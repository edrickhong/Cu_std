#pragma once

//NOTE: syscall always clobbers rax(res) rcx(ret addr) and r11(rflags)

#define _sys_exit(code) __asm__ volatile ( \
"syscall\n" \
: \
: [call] "a" (60), [c] "D" ((u64)code) \
: "rcx", "r11" \
)


extern "C" u32 Linux_Syscall_Clone(u32 clone_flags,void* newsp,void* parent_tid,void* child_tid);

#define _sys_clone(clone_flags,newsp,parent_tid,child_tid) Linux_Syscall_Clone(clone_flags,newsp,parent_tid,child_tid)

#define _sys_mmap(addr,len,prot,flags,fd,off,retptr) __asm__ volatile ( \
"movq %[fl],%%r10\n" \
"movq %[f],%%r8\n" \
"movq %[of],%%r9\n" \
"syscall\n" \
"movq %%rax,%[r]\n" \
: [r] "=g" (retptr) \
: [call] "a" (9), [ad] "D" (addr), [ln] "S" ((u64)len), [prt] "d" ((u64)prot), [fl] "g" ((u64)flags), [f] "g"  ((u64)fd), [of] "g" ((u64)off) \
: "%r10", "%r8", "%r9", "rcx", "r11" \
)

#define _sys_munmap(addr,len,ret32) __asm__ volatile ( \
"syscall\n" \
"movl %%eax,%[r]\n" \
: [r] "=g" (ret32) \
: [call] "a" (11), [ad] "D" (addr), [ln] "S" ((u64)len) \
: "rcx", "r11" \
)

#define _sys_futex(addr,op,val,timespec_val2,addr2,val3,ret32) __asm__ volatile ( \
"movq %[fl],%%r10\n" \
"movq %[f],%%r8\n" \
"movl %[of],%%r9d\n" \
"syscall\n" \
"movl %%eax,%[r]\n" \
: [r] "=g" (ret32) \
: [call] "a" (202), [ad] "D" (addr), [ln] "S" ((u32)op), [prt] "d" ((u32)val), [fl] "g" ((u64)timespec_val2), [f] "g"  (addr2), [of] "g" ((u32)val3) \
: "%r10", "%r8", "%r9", "memory", "rcx", "r11" \
)


#define _sys_gettid(ret32) __asm__ volatile ( \
"syscall\n" \
"movl %%eax,%[r]\n" \
: [r] "=g" (ret32) \
: [call] "a" (186) \
: "rcx", "r11" \
)