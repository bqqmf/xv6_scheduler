#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
extern struct {
    struct spinlock lock;
    struct proc proc[NPROC];
} ptable;
extern int time_slice[NQUEUE];
extern int q_size[NQUEUE];
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
int total_cpu_used = 0; // sum of all proc's cpu_used

// ojh
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;

      struct proc *p = myproc();
      // ojh
      if (p && p->pid > 2) {
          p->cpu_burst ++;
          p->cpu_used++;
          total_cpu_used ++;

          // 이 부분을 학생들은 debug=1에 넣어야 함.
          if (p->pid > 3 && p->cpu_used == p->end_time) {
              cprintf("PID: %d uses %d ticks in mlfq[%d], total(%d/%d)\n",
                    p->pid, p->cpu_burst,
                    p->q_lv, p->cpu_used, p->end_time);
              cprintf("PID: %d, used %d ticks. terminated\n", p->pid, p->cpu_used);  
              kill(p->pid);
          }
      }

      wakeup(&ticks);
      release(&tickslock);
    }

    // ojh
    acquire(&ptable.lock);
    increase_waits();
    check_aging();
    release(&ptable.lock);

    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  // ojh
  struct proc *p = myproc();
  if(p && p->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER){
      if (p->pid > 3 && p->cpu_burst == p->time_slice) {
          acquire(&tickslock);

          // 이 부분을 학생들은 debug=1에 넣어야 함.
          cprintf("PID: %d uses %d ticks in mlfq[%d], total(%d/%d)\n",
                  p->pid, p->cpu_burst,
                  p->q_lv, p->cpu_used, p->end_time);

          release(&tickslock);
          yield();
      }
  }
  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
