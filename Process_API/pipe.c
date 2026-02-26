#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static void die(const char *msg){ perror(msg); exit(1); }

int main(void) {
    int p[2];
    if (pipe(p) < 0) die("pipe");

    int rc = fork();
    if (rc < 0) die("fork");

    if (rc == 0) {
        // child
        close(p[0]); // read end close
        printf("hello\n");
        fflush(stdout);
        // 부모에게 "출력 끝" 신호 1바이트
        if (write(p[1], "X", 1) != 1) die("write");
        close(p[1]);
        exit(0);
    } else {
        // parent
        close(p[1]); // write end close
        char buf;
        // 자식이 신호 보낼 때까지 블록 -> 순서 보장
        if (read(p[0], &buf, 1) != 1) die("read");
        close(p[0]);
        printf("goodbye\n");
    }
    return 0;
}