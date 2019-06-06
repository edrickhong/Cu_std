#pragma once

#define _sys_exit(code) __asm__ volatile ( \
"syscall\n" \
: \
: [call] "a" (60), [c] "D" ((u64)code) \
)

#define _sys_clone(clone_flags,newsp,parent_tid,child_tid,ret) __asm__ volatile ( \
"movq %[ct],%%r10\n" \
"syscall\n" \
"movq %%rax,%[r]\n" \
: [r] "=g" (ret) \
: [call] "a" (56), [cf] "D" (clone_flags), [ns] "S" ((u64)newsp), [pt] "d" ((u64)parent_tid), [ct] "g" ((u64)child_tid) \
)

#define _sys_mmap(addr,len,prot,flags,fd,off,ret) __asm__ volatile ( \
"movq %[fl],%%r10\n" \
"movq %[f],%%r8\n" \
"movq %[of],%%r9\n" \
"syscall\n" \
"movq %%rax,%[r]\n" \
: [r] "=g" (ret) \
: [call] "a" (9), [ad] "D" (addr), [ln] "S" ((u64)len), [prt] "d" ((u64)prot), [fl] "g" ((u64)flags), [f] "g"  ((u64)fd), [of] "g" ((u64)off) \
)

#define _sys_munmap(addr,len) __asm__ volatile ( \
"syscall\n" \
: \
: [call] "a" (11), [ad] "D" (addr), [ln] "S" ((u64)len) \
)

