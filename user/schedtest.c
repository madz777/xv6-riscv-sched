#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"

// find prime numbers
// https://www.codesdope.com/blog/article/prime-numbers-using-sieve-algorithm-in-c/
void sieve_of_eratosthenes(int n) {
    int *primes = (int*)malloc(sizeof(int)*n+1);
    int i, j;
    for(i = 2; i<=n; i++) {
        primes[i] = i;
    }
    i = 2;
    while ((i*i) <= n)
    {
        if (primes[i] != 0)
        {
            for(j=2; j<n; j++)
            {
                if (primes[i]*j > n)
                    break;
                else
                    // Instead of deleteing, making elemnets 0
                    primes[primes[i]*j]=0;
            }
        }
        i++;
    }
    free(primes);
}

void heavy_io() {
    char *fname = "/data";
    for(int repeat = 0; repeat < 15; repeat++) {
        int fd = open(fname, O_RDWR|O_CREATE);
        char data[100]; // arbitrary data
        for(int i = 0; i < 30; i++) {
            write(fd, data, 100);
        }
        close(fd);
    }
}

void heavy_io_with_sieve() {
    char *fname = "/data";
    for(int repeat = 0; repeat < 5; repeat++) {
        int fd = open(fname, O_RDWR|O_CREATE);
        char data[100]; // arbitrary data
        for(int i = 0; i < 20; i++) {
            write(fd, data, 100);
            sieve_of_eratosthenes((i+1)*1000);
        }
        close(fd);
    }
}

void run(int processes, enum SchedulerChoice sc) {
    // use FIFO first so schedtest gets everything started before the children run
    set_scheduler(FIFO);
    /*
    int total_turnaround_time = 0;
    int total_response_time = 0;
    */
    // launch all processes
    for(int i = 0; i < processes; i++) {
        // make sure some processes are created a bit later than others
        // (don't sleep() because we don't want anyone else scheduled)
        for(int j = 0; j < 10000000; j++) {}
        printf("Forking #%d...\n", i);
        if(fork() == 0) {
            int pid = getpid();
            if(i < processes/3) {
                printf("Starting sieve %d...\n", pid);
                for(int k = 0; k < 50; k++) {
                    sieve_of_eratosthenes((i+1)*1000000);
                }
                printf("Ending sieve %d.\n", pid);
            } else if(i < 2*processes/3) {
                printf("Starting IO %d...\n", pid);
                heavy_io();
                printf("Ending IO %d.\n", pid);
            } else {
                printf("Starting sieve+IO %d...\n", pid);
                heavy_io_with_sieve();
                printf("Ending sieve+IO %d.\n", pid);
            }
            exit(0);
        }
    }
    // set scheduler after processes have been created but before the start running (mostly)
    set_scheduler(sc);
    // collect all processes
    /*
    int ctime, stime, etime, rtime, pid;
    */
    for(int i = 0; i < processes; i++) {
        wait(0);
        /*
        pid = wait_stats(&ctime, &stime, &etime, &rtime);
        printf("Child %d exited, ctime: %d, stime: %d, etime: %d, rtime: %d\n",
               pid, ctime, stime, etime, rtime);
        total_turnaround_time += (etime-ctime);
        total_response_time += (stime-ctime);
        */
    }
    /*
    printf("Total turnaround time: %d, avg: %d\n", total_turnaround_time, (total_turnaround_time / processes));
    printf("Total response time: %d, avg: %d\n", total_response_time, (total_response_time / processes));
    printf("\n");
    */
}

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Usage: schedtest <num of processes>\n");
        exit(0);
    }
    int processes = atoi(argv[1]);
    printf("Running %d processes\n", processes);

    printf("With scheduler RR\n");
    run(processes, RR);

    printf("With scheduler FIFO\n");
    run(processes, FIFO);

    /*
    printf("With scheduler LIFO\n");
    run(processes, LIFO);

    
    printf("With scheduler FAIR\n");
    run(processes, FAIR);
    */

    exit(0);
}

