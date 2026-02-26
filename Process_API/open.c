#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

static void die(const char *msg) { perror(msg); exit(1); }

int main(void) {
    int fd = open("hw2.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) die("open");

    int rc = fork();
    if (rc < 0) die("fork");

    const char *msg = (rc == 0) ? "child\n" : "parent\n";

    // 일부러 여러 번 써서 섞임을 관찰
    for (int i = 0; i < 5; i++) {
        if (write(fd, msg, strlen(msg)) < 0) die("write");
        usleep(10000); // 10ms
    }

    if (rc == 0) {
        close(fd);
        exit(0);
    } else {
        wait(NULL);
        close(fd);
        printf("Done. Check hw2.txt\n");
    }
    return 0;
}