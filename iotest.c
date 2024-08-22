#include "types.h"
#include "stat.h"
#include "user.h"

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
  
  for (i = 0; i < 3; i++) {
    pid = fork();
    if (pid == 0) { 
      io_bound_task(50);  
      printf(1, "IO-bound Process %d finished\n", i);
      exit();
    }
  }

  for (i = 0; i < 3; i++) {
    wait();
  }

  printf(1, "All processes finished\n");
  exit();
}

