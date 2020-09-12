#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

static uint64_t cycle_start;
static clock_t clock_start;
static struct timespec run_start;

#include <sched.h>
/* Set the process run on cpu */
int set_cpu(int cpu) {
    cpu_set_t cpu_mask;
    CPU_ZERO(&cpu_mask);
    CPU_SET(cpu, &cpu_mask);
    return sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);
}

/* Get the cpu timestamps count register value */
static uint64_t get_cycle() {
    uint32_t a, b;
    asm (
            "rdtsc; \n\t"
            "movl %%edx, %0; \n\t"
            "movl %%eax, %1; \n\t"
            : "=r"(a), "=r"(b)
            :
            : "%edx", "%eax"
        );
//    printf("0x%08x 0x%08x \n", a, b);
    return ((((uint64_t)a) << 32) | b);
}

 /* Start count the cpu cycle */
void start_cycle_counter() {
    cycle_start = get_cycle();
}

/* Get the cpu cycles from last start_cycle_counter() running */
double get_cycle_counter(int p) {
    uint64_t cycle_end = get_cycle();
    double ret = 0.0;
    if (cycle_end < cycle_start) {
        ret = ((1l << 63) - cycle_end + cycle_start + (1l << 63));
    }
    else {
        ret = (cycle_end - cycle_start);
    }
    if (ret < 0) {
        fputs("count cycles error!", stderr);
        exit(-1);
    }
    if (p) {
        printf("This code segment runs %f cpu cycles.\n", ret);
    }
    return ret;
}


/* Start count the cpu real time */
void start_clock_counter() {
    clock_start = clock();
}

/* Get the real time from last start_clock_counter() running */
double get_clock_counter(int p) {
    clock_t clock_end = clock();
    double ret = clock_end - clock_start;
    ret = ret / CLOCKS_PER_SEC * 1000.0;
    if (p) {
        printf("This code segment runs %f ms real time.\n", ret);
    }
    return ret;
}

/* Start count the time from start to end on computer */
void start_run_counter() {
    clock_gettime(CLOCK_MONOTONIC, &run_start);
}

/* Get the program time from last start_run_counter() running */
double get_run_counter(int p) {
    struct timespec run_end;
    clock_gettime(CLOCK_MONOTONIC, &run_end);
    double ret = (run_end.tv_sec - run_start.tv_sec) * 1000.0 + (run_end.tv_nsec - run_start.tv_nsec) / 1000000.0;
    if (p) {
        printf("This code segment runs %f ms.\n", ret);
    }
    return ret;
}

/* Get the real cpu running frequency */
double mhz(int p) {
    start_cycle_counter();
    sleep(1);
    double cycles = get_cycle_counter(0) / 1000000.0;
    if (p) {
        printf("The frequecy of this CPU is %0.3f mHZ.\n", cycles);
    }
    return cycles;
}
