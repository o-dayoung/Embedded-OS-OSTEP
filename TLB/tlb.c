// tlb.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

static inline double now_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1e6 + (double)tv.tv_usec;
}

// 컴파일러가 루프를 지워버리지 못하게
static volatile int sink = 0;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <num_pages> <trials>\n", argv[0]);
        return 1;
    }

    int num_pages = atoi(argv[1]);
    int trials    = atoi(argv[2]);

    long page_size = sysconf(_SC_PAGESIZE);
    int jump = (int)(page_size / (int)sizeof(int));   // 페이지당 int 개수

    size_t nints = (size_t)num_pages * (size_t)jump;  // 총 int 개수
    size_t bytes = nints * sizeof(int);

    // 페이지 경계 정렬(측정 안정화)
    size_t aligned_bytes = ((bytes + page_size - 1) / page_size) * page_size;
    int *a = (int*)aligned_alloc((size_t)page_size, aligned_bytes);
    if (!a) { perror("aligned_alloc"); return 1; }

    // 첫 접근(페이지 폴트/캐시 워밍업) 영향 줄이기
    for (size_t i = 0; i < nints; i += (size_t)jump) a[i] = 0;

    double t0 = now_us();

    for (int t = 0; t < trials; t++) {
        for (size_t i = 0; i < nints; i += (size_t)jump) {
            a[i] += 1;
        }
    }

    double t1 = now_us();

    double accesses = (double)trials * (double)num_pages;   // 총 “페이지 접근” 횟수
    double total_us = (t1 - t0);
    double avg_ns = (total_us * 1000.0) / accesses;         // us → ns

    sink = a[0]; // 결과 사용 흔적 남기기
    printf("pages=%d trials=%d avg=%.3f ns/access (total=%.3f ms)\n",
           num_pages, trials, avg_ns, total_us / 1000.0);

    free(a);
    return 0;
}