#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define NPROC 12 
#define INPUT_FILENAME "README"
#define OUTPUT_FILENAME1 "io_test1.txt"
#define OUTPUT_FILENAME2 "io_test2.txt"
#define BUFFER_SIZE 1024

void cpu_bound_task(int n) {
    int i;
    volatile int sum = 0;
    for (i=0; i<30000000; i++)
        sum += i;

}
void cpu_bound_task2(int ticks) {
    int i;
    volatile int sum = 0;
    int start_time, current_time, elapsed_time;
    start_time = uptime();

    for(;;) {

        for (i = 0; 100000; i++) {
            sum += i;

        }
        current_time = uptime();
        elapsed_time = current_time - start_time;
        printf(1, "start : %d cur : %d, ela : %d\n", start_time, current_time, elapsed_time);

        if (elapsed_time >= ticks) 
            break;
    }
}

void io_bound_task(int T, const char *input, const char *output) {
    int input_fd, output_fd;
    char buffer[BUFFER_SIZE];
    int n;

    for(int i = 0; i < T; i++){
        if ((input_fd = open(input, O_RDONLY)) < 0) {
            printf(2, "Failed to open %s for reading\n", input);
            exit();
        }

        if ((output_fd = open(output, O_CREATE | O_WRONLY)) < 0) {
            printf(2, "Failed to open %s for reading\n", output);
            exit();
        }

        while ((n = read(input_fd, buffer, sizeof(buffer))) > 0) {
            if (write(output_fd, buffer, n) < 0) {
                printf(2, "Failed to write to %d file\n", output);
                close(input_fd);
                close(output_fd);
                exit();
            }
        }
        if (n < 0) {
            printf(2, "Failed to read from %s file\n", input);
        }
        close(input_fd);
        close(output_fd);
    }
}




int main(void) {
    int pid;
    int i;

    for (i = 0; i < NPROC; i++) {
        pid = fork();
        if (pid == 0) { // 자식 프로세스
                        //printf(1, "\nProcess %d created.\n", i);
            if (i % 3 == 0) {
                printf(1, "CPU-bound Process %d created\n", i);
                cpu_bound_task(100 * (i + 1));  // CPU-bound 작업
                printf(1, "CPU-bound Process %d finished\n", i);
                exit();
            } else if (i % 3 == 2) {  
                printf(1, "IO-bound Process %d created\n", i);
                io_bound_task(1, INPUT_FILENAME, OUTPUT_FILENAME2);
                printf(1, "IO-bound Process %d finished\n", i);
                exit();
            } else {
                printf(1, "CPU + IO Process %d created\n", i);
                cpu_bound_task(100);  
                io_bound_task(1, INPUT_FILENAME, OUTPUT_FILENAME1);
                printf(1, "CPU + IO Process %d finished\n", i);
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

