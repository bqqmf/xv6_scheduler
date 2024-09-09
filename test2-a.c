#include "types.h"
#include "user.h"
#include "stat.h"

#define PNUM 3
int q_lv[PNUM] = { 3, 3, 3 };
int cpu_burst[PNUM] = { 0, 0, 0 };
int cpu_wait[PNUM] = { 0, 0, 0 };
int io_wait_time[PNUM] = { 5, 0, 0 };
int end_time[PNUM] = { 500, 500, 500 };

void scheduler_func(void)
{
    int pid;
    printf(1, "start scheduler_test\n");
    for (int i = 0; i < 2; i++) {
        pid = fork();
        if (pid == 0) {
            set_proc_info(q_lv[i], cpu_burst[i], cpu_wait[i], io_wait_time[i], end_time[i]);
            while (1)
                ;
            printf(1, "PID: %d terminated\n", getpid());
            exit();
        } 
    }
    for (int i = 0; i < PNUM; i++)
        wait();
    printf(1, "end of scheduelr_test\n");
}

int main(void)
{
    scheduler_func();
    exit();
}

