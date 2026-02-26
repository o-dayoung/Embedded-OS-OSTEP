#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int main(void) {
    int rc = fork();
    if (rc < 0) { perror("fork"); exit(1); }

    if (rc == 0) {
        // child: try wait (should fail)
        int status;
        pid_t w = wait(&status);
        printf("child wait() returned %d, errno=%d\n", (int)w, errno);
        exit(0);
    } else {
        int status;
        pid_t w = wait(&status);
        printf("parent wait() returned %d (child pid=%d)\n", (int)w, rc);

        if (WIFEXITED(status)) {
            printf("child exit code=%d\n", WEXITSTATUS(status));
        }
    }
    return 0;
}