#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>

int main() {
    int pipe1[2], pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    const int iterations = 10000;
    char buf = 'a';

    if (pid == 0) {
        // child: pipe1에서 읽고, pipe2로 씀
        close(pipe1[1]);
        close(pipe2[0]);

        while (1) {
            if (read(pipe1[0], &buf, 1) != 1) break;
            if (write(pipe2[1], &buf, 1) != 1) break;
        }
        return 0;

    } else {
        // parent: pipe1로 쓰고, pipe2에서 읽음
        close(pipe1[0]);
        close(pipe2[1]);

        struct timeval start, end;
        gettimeofday(&start, NULL);

        for (int i = 0; i < iterations; i++) {
            write(pipe1[1], &buf, 1);
            read(pipe2[0], &buf, 1);
        }

        gettimeofday(&end, NULL);

        long total_us = (end.tv_sec - start.tv_sec) * 1000000L
                      + (end.tv_usec - start.tv_usec);

        // 왕복 1회에 문맥교환 2번 발생(부모->자식, 자식->부모)으로 보고 2로 나눔
        double per_switch = (double) total_us / (iterations * 2);

        printf("Total time: %ld microseconds\n", total_us);
        printf("Estimated time per context switch: %.6f microseconds\n", per_switch);

        // child 종료
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);

        return 0;
    }
}