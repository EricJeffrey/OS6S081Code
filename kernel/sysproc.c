#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  backtrace();

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_sigalarm(void)
{
  int tick_interval;
  uint64 handler;
  if (argint(0, &tick_interval) < 0 || argaddr(1, &handler) < 0)
    return -1;
  struct proc *p = myproc();
  // cancel alarm
  if (tick_interval == 0) {
    p->alarmctx.tick_interval = 0;
    p->alarmctx.handler = 0;
  } else {
    p->alarmctx.tick_interval = tick_interval;
    p->alarmctx.handler = ((void *)handler);
  }
  return 0;
}

uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();
  if (p->alarmctx.alarming == 1) {
    p->alarmctx.alarming = 0;
    p->alarmctx.tick_last_call = 0;
    *(p->trapframe) = p->alarmctx.alarmframe;
    memset(&p->alarmctx.alarmframe, 0, sizeof(p->alarmctx.alarmframe));
  }
  return 0;
}
