#include "types.h"
#include "stat.h"
#include "user.h"

void cpu_bound_task(int loops) {
  int i;
  for (i = 0; i < loops; i++) {
    asm volatile("nop");
  }
}

int main(void) {
  int pid;
  int i;
  
  for (i = 0; i < 3; i++) {
    pid = fork();
    if (pid == 0) { // 자식 프로세스
      cpu_bound_task(100000000 * (i + 1));  // CPU-bound 작업
      printf(1, "CPU-bound Process %d finished\n", i);
      exit();
    }
  }

  for (i = 0; i < 3; i++) {
    wait();
  }

  printf(1, "All processes finished\n");
  exit();
}

