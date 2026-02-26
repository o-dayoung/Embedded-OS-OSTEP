#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static void die(const char *m){ perror(m); exit(1); }

int main(void) {
    int p[2];
    if (pipe(p) < 0) die("pipe");

    pid_t c1 = fork();
    if (c1 < 0) die("fork c1");

    if (c1 == 0) {
        // child1: ls -> pipe write
        dup2(p[1], STDOUT_FILENO);
        close(p[0]); close(p[1]);
        execlp("ls", "ls", (char*)NULL);
        die("execlp ls");
    }

    pid_t c2 = fork();
    if (c2 < 0) die("fork c2");

    if (c2 == 0) {
        // child2: wc -l <- pipe read
        dup2(p[0], STDIN_FILENO);
        close(p[0]); close(p[1]);
        execlp("wc", "wc", "-l", (char*)NULL);
        die("execlp wc");
    }

    // parent: close pipe ends and wait
    close(p[0]); close(p[1]);
    waitpid(c1, NULL, 0);
    waitpid(c2, NULL, 0);
    return 0;
}