#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int c1 = fork();
    if (c1 < 0) { perror("fork"); exit(1); }
    if (c1 == 0) { sleep(1); exit(11); }

    int c2 = fork();
    if (c2 < 0) { perror("fork"); exit(1); }
    if (c2 == 0) { sleep(2); exit(22); }

    int status;
    pid_t w = waitpid(c2, &status, 0); // c2만 기다림
    printf("waitpid waited pid=%d\n", (int)w);
    if (WIFEXITED(status)) printf("exit=%d\n", WEXITSTATUS(status));

    // 남은 자식 정리
    waitpid(c1, NULL, 0);
    return 0;
}