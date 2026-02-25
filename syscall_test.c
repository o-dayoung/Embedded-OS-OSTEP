#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

int main() {
    struct timeval start, end;
    int i;
    int iterations = 1000000;

    gettimeofday(&start, NULL);

    for(i = 0; i < iterations; i++) {
        getpid();
    }

    gettimeofday(&end, NULL);

    long seconds = end.tv_sec - start.tv_sec;
    long useconds = end.tv_usec - start.tv_usec;
    long total_time = seconds * 1000000 + useconds;

    printf("Total time: %ld microseconds\n", total_time);
    printf("Time per syscall: %f microseconds\n",
           (double) total_time / iterations);

    return 0;
}