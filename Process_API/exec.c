#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

static void usage(const char *p) {
    fprintf(stderr, "usage: %s [execl|execle|execlp|execv|execvp|execve]\n", p);
    exit(1);
}

int main(int argc, char *argv[], char *envp[]) {
    if (argc != 2) usage(argv[0]);

    int rc = fork();
    if (rc < 0) { perror("fork"); exit(1); }

    if (rc == 0) {
        // child: run /bin/ls using selected exec*
        if (!strcmp(argv[1], "execl")) {
            execl("/bin/ls", "ls", "-l", (char*)NULL);
        } else if (!strcmp(argv[1], "execle")) {
            char *const newenv[] = { "MYVAR=HELLO", NULL };
            execle("/bin/ls", "ls", "-l", (char*)NULL, newenv);
        } else if (!strcmp(argv[1], "execlp")) {
            // PATH에서 ls 검색
            execlp("ls", "ls", "-l", (char*)NULL);
        } else if (!strcmp(argv[1], "execv")) {
            char *const args[] = { "ls", "-l", NULL };
            execv("/bin/ls", args);
        } else if (!strcmp(argv[1], "execvp")) {
            char *const args[] = { "ls", "-l", NULL };
            execvp("ls", args); // PATH 검색
        } else if (!strcmp(argv[1], "execve")) {
            char *const args[] = { "ls", "-l", NULL };
            char *const newenv[] = { "MYVAR=HELLO", NULL };
            execve("/bin/ls", args, newenv);
        } else {
            usage(argv[0]);
        }

        // exec 성공하면 여기로 절대 안 옴
        perror("exec failed");
        exit(1);
    } else {
        wait(NULL);
    }
    return 0;
}