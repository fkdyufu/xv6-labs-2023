#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
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
  uint64 a, start, last;
  int size;
  uint64 uabits;
  pte_t *pte;
  struct proc *p;
  unsigned int kabits;
  int flag;

  argaddr(0, &a);
  argint(1, &size);
  argaddr(2, &uabits);
  kabits = 0;
  flag = 1;
  last = a + ((size - 1) * PGSIZE);
  p = myproc();

  if(size > MAXDETECTPG)
    return -1;

  for(start = a; start <= last; flag <<= 1, start += PGSIZE){
    pte = walk(p->pagetable, start, 0);
    if(*pte & PTE_A){
      kabits = kabits | flag;
      *pte = *pte & (~PTE_A);
    }
  }

  copyout(p->pagetable, (uint64)uabits, (char*)&kabits, MAXDETECTPG);
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
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
