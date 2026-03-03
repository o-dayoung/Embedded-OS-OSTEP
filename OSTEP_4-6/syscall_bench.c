// syscall_bench.c
// OSTEP 4-6: 시스템 콜 오버헤드 계측
//
// ======================
// ✅ 반영된 요구사항
// 1) getpid() / read(fd,NULL,0) 각각 1,000,000회 반복 호출
// 2) 평균 비용을 마이크로초(us) 단위로 계산
// 3) 최소 5회 반복 실행 후 평균 및 표준편차 출력
// 4) gettimeofday() vs clock_gettime() 비교
// 5) getpid() vs read() 비교
// ======================
//
// 컴파일: gcc -O2 -Wall -Wextra -o bench syscall_bench.c
// 실행:   ./bench
//

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define NLOOPS_DEFAULT 1000000   // 요구사항: 1,000,000회 반복
#define NRUNS_DEFAULT 5          // 요구사항: 최소 5회 반복

// 컴파일러 최적화 방지용 변수
// (루프 내부 코드가 제거되지 않도록 하기 위함)
static volatile unsigned long sink_ul = 0;

//////////////////////////////////////////////////////////////
// 📌 시간 측정 함수 1
// 요구사항: gettimeofday() 사용한 측정 비교
//////////////////////////////////////////////////////////////
static long long now_ns_gettimeofday(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    // 초 → 나노초 변환
    return (long long)tv.tv_sec * 1000000000LL +
           (long long)tv.tv_usec * 1000LL;
}

//////////////////////////////////////////////////////////////
// 📌 시간 측정 함수 2
// 요구사항: clock_gettime() 사용한 측정 비교
//////////////////////////////////////////////////////////////
static long long now_ns_clock_gettime(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL +
           (long long)ts.tv_nsec;
}

typedef long long (*now_ns_fn)(void);

//////////////////////////////////////////////////////////////
// 📌 측정 대상 구분
// 요구사항: getpid() vs read() 비교
//////////////////////////////////////////////////////////////
typedef enum {
    OP_EMPTY,   // 루프 자체 오버헤드 측정용
    OP_GETPID,  // getpid() 측정
    OP_READ0    // read(fd,NULL,0) 측정
} op_t;

//////////////////////////////////////////////////////////////
// 📌 단일 실험 실행 함수
// → 1,000,000회 반복
// → 평균 us 계산
//////////////////////////////////////////////////////////////
static double bench_once_us(now_ns_fn timer,
                            op_t op,
                            int nloops,
                            int fd)
{
    //////////////////////////////////////////////////////
    // 워밍업 루프 (캐시/분기 예측 안정화)
    //////////////////////////////////////////////////////
    for (int i = 0; i < 10000; i++) {
        if (op == OP_GETPID)
            sink_ul += getpid();
        else if (op == OP_READ0)
            sink_ul += read(fd, NULL, 0);
        else
            sink_ul += i;
    }

    //////////////////////////////////////////////////////
    // 실제 측정 시작
    //////////////////////////////////////////////////////
    long long start = timer();

    for (int i = 0; i < nloops; i++) {

        //////////////////////////////////////////////////
        // 요구사항: getpid() 측정
        //////////////////////////////////////////////////
        if (op == OP_GETPID) {
            sink_ul += getpid();
        }

        //////////////////////////////////////////////////
        // 요구사항: read(fd,NULL,0) 측정
        //////////////////////////////////////////////////
        else if (op == OP_READ0) {
            sink_ul += read(fd, NULL, 0);
        }

        //////////////////////////////////////////////////
        // 루프 오버헤드 측정
        //////////////////////////////////////////////////
        else {
            sink_ul += i;
        }
    }

    //////////////////////////////////////////////////////
    // 측정 종료
    //////////////////////////////////////////////////////
    long long end = timer();

    long long diff_ns = end - start;

    //////////////////////////////////////////////////////
    // 요구사항: 평균 비용을 마이크로초(us) 단위로 계산
    //////////////////////////////////////////////////////
    return (double)diff_ns / nloops / 1000.0;
}

//////////////////////////////////////////////////////////////
// 📌 평균 및 표준편차 계산
// 요구사항: 5회 반복 후 평균값 도출
//////////////////////////////////////////////////////////////
static void mean_stddev(double *arr,
                        int n,
                        double *mean,
                        double *stddev)
{
    double m = 0;
    for (int i = 0; i < n; i++)
        m += arr[i];

    m /= n;

    double var = 0;
    for (int i = 0; i < n; i++) {
        double d = arr[i] - m;
        var += d * d;
    }

    var /= n;

    *mean = m;
    *stddev = sqrt(var);
}

//////////////////////////////////////////////////////////////
// 📌 하나의 타이머에 대해 전체 비교 수행
// (gettimeofday / clock_gettime 각각 실행)
//////////////////////////////////////////////////////////////
static void run_suite(now_ns_fn timer,
                      const char* timer_name,
                      int nloops,
                      int nruns,
                      int fd)
{
    op_t ops[] = { OP_EMPTY, OP_GETPID, OP_READ0 };

    printf("\n=== [%s] 기준 측정 ===\n", timer_name);

    for (int k = 0; k < 3; k++) {

        double samples[NRUNS_DEFAULT];

        //////////////////////////////////////////////////
        // 요구사항: 최소 5회 반복 실행
        //////////////////////////////////////////////////
        for (int r = 0; r < nruns; r++) {
            samples[r] = bench_once_us(timer,
                                       ops[k],
                                       nloops,
                                       fd);

            printf("Run %d: %.3f us\n",
                   r + 1,
                   samples[r]);
        }

        //////////////////////////////////////////////////
        // 평균 및 표준편차 계산
        //////////////////////////////////////////////////
        double mean, stddev;
        mean_stddev(samples, nruns, &mean, &stddev);

        printf("평균: %.3f us (표준편차: %.3f us)\n\n",
               mean, stddev);
    }
}

//////////////////////////////////////////////////////////////
// 📌 main 함수
//////////////////////////////////////////////////////////////
int main() {

    int nloops = NLOOPS_DEFAULT; // 1,000,000회
    int nruns  = NRUNS_DEFAULT;  // 5회 반복

    //////////////////////////////////////////////////////
    // read() 측정을 위한 fd 생성
    //////////////////////////////////////////////////////
    int fd = open("/dev/null", O_RDONLY);
    if (fd < 0) {
        perror("open 실패");
        return 1;
    }

    printf("=== OSTEP 시스템콜 오버헤드 실험 ===\n");
    printf("반복 횟수: %d\n", nloops);
    printf("실행 반복: %d회\n", nruns);

    //////////////////////////////////////////////////////
    // 요구사항: gettimeofday vs clock_gettime 비교
    //////////////////////////////////////////////////////
    run_suite(now_ns_gettimeofday,
              "gettimeofday()",
              nloops,
              nruns,
              fd);

    run_suite(now_ns_clock_gettime,
              "clock_gettime()",
              nloops,
              nruns,
              fd);

    close(fd);

    return 0;
}