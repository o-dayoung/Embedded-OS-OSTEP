#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int rc = fork();
    if (rc < 0) { perror("fork"); exit(1); }

    if (rc == 0) {
        close(STDOUT_FILENO);
        // stdout은 닫혔지만 버퍼에 써질 수는 있어 보여서 flush까지 해봄
        printf("child: will this print?\n");
        fflush(stdout);
        // stderr는 2번이라 살아있음
        fprintf(stderr, "child: stdout closed, but stderr works\n");
        exit(0);
    } else {
        wait(NULL);
        printf("parent: stdout is fine\n");
    }
    return 0;
}