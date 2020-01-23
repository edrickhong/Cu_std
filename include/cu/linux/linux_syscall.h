#pragma once

//TODO: should we care about type safety?
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

#define _sys_setaffinity(pid,len,mask_ptr,ret32) __asm__ volatile ( \
"syscall\n" \
"movl %%eax,%[r]\n" \
: [r] "=g" (ret32) \
: [call] "a" (203), [p] "D" (pid), [ln] "S" ((u32)len), [mask] "d" (mask_ptr) \
: "memory", "rcx", "r11" \
)

#define _sys_getaffinity(pid,len,mask_ptr,ret32) __asm__ volatile ( \
"syscall\n" \
"movl %%eax,%[r]\n" \
: [r] "=g" (ret32) \
: [call] "a" (204), [p] "D" (pid), [ln] "S" ((u32)len), [mask] "d" (mask_ptr) \
: "memory", "rcx", "r11" \
)


#define _sys_sched_setparam(pid,param,ret32) __asm__ volatile ( \
"syscall\n" \
"movl %%eax,%[r]\n" \
: [r] "=g" (ret32) \
: [call] "a" (142), [p] "D" (pid), [p] "S" (param) \
: "memory", "rcx", "r11" \
)

#define _sys_sched_getparam(pid,param,ret32) __asm__ volatile ( \
"syscall\n" \
"movl %%eax,%[r]\n" \
: [r] "=g" (ret32) \
: [call] "a" (143), [p] "D" (pid), [p] "S" (param) \
: "memory", "rcx", "r11" \
)

u32 _ainline Linux_Syscall_Get_Priority_Max(u32 policy){
    
    switch(policy){
        case SCHED_FIFO: //fallthrough
        case SCHED_RR:{
            return 99;
        }break;
        
        case SCHED_IDLE: //fallthrough
        case SCHED_NORMAL: //fallthrough
        case SCHED_BATCH: //fallthrough
        case SCHED_DEADLINE:{
            return 0;
        }break;
    }
    
    return (u32)-1;
}

u32 _ainline Linux_Syscall_Get_Priority_Min(u32 policy){
    switch(policy){
        case SCHED_FIFO: //fallthrough
        case SCHED_RR:{
            return 1;
        }break;
        
        case SCHED_IDLE: //fallthrough
        case SCHED_NORMAL: //fallthrough
        case SCHED_BATCH: //fallthrough
        case SCHED_DEADLINE:{
            return 0;
        }break;
    }
    
    return (u32)-1;
}

#define _syscall_sched_get_priority_min(p) Linux_Syscall_Get_Priority_Min(p)
#define _syscall_sched_get_priority_max(p) Linux_Syscall_Get_Priority_Max(p)

#define _sys_sched_rr_get_interval(pid,timespec_inter,ret32) __asm__ volatile ( \
"syscall\n" \
"movl %%eax,%[r]\n" \
: [r] "=g" (ret32) \
: [call] "a" (148), [p] "D" (pid), [t] "S" (timespec_inter) \
: "memory", "rcx", "r11" \
)

#define _sys_sched_setattr(pid,attr,flags,ret32) __asm__ volatile ( \
"syscall\n" \
"movl %%eax,%[r]\n" \
: [r] "=g" (ret32) \
: [call] "a" (314), [p] "D" (pid), [at] "S" (attr), [f] "d" ((u32)flags) \
: "memory", "rcx", "r11" \
)

#define _sys_sched_getattr(pid,attr,size,flags,ret32) __asm__ volatile ( \
"movl %[f],%%r10d\n" \
"syscall\n" \
"movl %%eax,%[r]\n" \
: [r] "=g" (ret32) \
: [call] "a" (315), [p] "D" (pid), [at] "S" (attr), [s] "d" (size), [f] "g" ((u32)flags) \
: "r10", "memory", "rcx", "r11" \
)


#define _sys_sched_setscheduler(pid,policy,param,ret32) __asm__ volatile ( \
"syscall\n" \
"movl %%eax,%[r]\n" \
: [r] "=g" (ret32) \
: [call] "a" (144), [p] "D" (pid), [pl] "S" ((u32)policy), [p] "d" (param) \
: "memory", "rcx", "r11" \
)


//only returns policy
#define _sys_sched_getscheduler(pid,ret32) __asm__ volatile ( \
"syscall\n" \
"movl %%eax,%[r]\n" \
: [r] "=g" (ret32) \
: [call] "a" (145), [p] "D" (pid) \
: "rcx", "r11" \
)

