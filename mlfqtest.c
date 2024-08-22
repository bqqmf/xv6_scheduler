#include "types.h"
#include "stat.h"
#include "user.h"

#define NPROC 4 

void cpu_bound_task(int loops) {
  int i;
  for (i = 0; i < loops; i++) {
    asm volatile("nop");
  }
}

void io_bound_task(int iterations) {
    int i;
    for (i = 0; i < iterations; i++) {
        sleep(10);
        asm volatile("nop");
    }
}
int main(void) {
  int pid;
  int i;
  
  for (i = 0; i < NPROC; i++) {
    pid = fork();
    if (pid == 0) { // 자식 프로세스
      //printf(1, "\nProcess %d created.\n", i);
      if (i % 2 == 0) {
        cpu_bound_task(200000000 * (i + 1));  // CPU-bound 작업
        printf(1, "CPU-bound Process %d finished\n", i);
        exit();
      } else {
        io_bound_task(50);
        printf(1, "IO-bound Process %d finished\n", i);
        exit();
      }
    }
  }

  for (i = 0; i < NPROC; i++) {
    wait();
  }

  printf(1, "All processes finished\n");
  exit();
}

