#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
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


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
#define MAX_NUM_PAGES (1024 * 8)
  uint64 va;
  int numofpages;
  char *outbitmask;
  if (argaddr(0, &va) < 0 || argint(1, &numofpages) < 0 ||
          argaddr(2, (uint64 *)(&outbitmask)) < 0)
    return -1;
  if (numofpages > MAX_NUM_PAGES)
    panic("sys_pgaccess: number of pages too large");

  int n = numofpages / 8 + ((numofpages % 8 > 0) ? 1 : 0);

  char bitmask[MAX_NUM_PAGES / 8];
  memset(bitmask, 0, MAX_NUM_PAGES / 8);

  struct proc *p = myproc();
  pagetable_t pagetable = p->pagetable;

  for (int i = 0; i < numofpages; i++) {
    uint64 curva = va + i * PGSIZE;
    pagetable_t curpgtbl = pagetable;
    int found = 1;
    for (int level = 2; level > 0; level--) {
      pte_t *pte = &curpgtbl[PX(level, curva)];
      if (*pte & PTE_V) {
        curpgtbl = (pagetable_t)PTE2PA(*pte);
      } else {
        found = 0;
        break;
      }
    }
    if (found) {
      pte_t *pte = &curpgtbl[PX(0, curva)];
      if ((*pte & PTE_V) && (*pte & PTE_A)) {
        bitmask[i / 8 ] |= (1 << (i % 8));
        *pte &= ~(PTE_A);
      }
    }
  }

  if (copyout(pagetable, (uint64)outbitmask, 
              bitmask, n) < 0) {
    panic("sys_pgaccess: copyout");
  }


  return 0;
#undef MAX_NUM_PAGES 
}
#endif

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
