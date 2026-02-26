#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int x = 100;
    printf("parent(before fork) pid=%d x=%d &x=%p\n", getpid(), x, (void*)&x);

    int rc = fork();
    if (rc < 0) { perror("fork"); exit(1); }

    if (rc == 0) {
        // child
        printf("child(after fork)  pid=%d x=%d &x=%p\n", getpid(), x, (void*)&x);
        x = 200;
        printf("child(changed)     pid=%d x=%d &x=%p\n", getpid(), x, (void*)&x);
        exit(0);
    } else {
        // parent
        x = 150;
        printf("parent(changed)    pid=%d x=%d &x=%p\n", getpid(), x, (void*)&x);
        wait(NULL);
        printf("parent(after wait) pid=%d x=%d &x=%p\n", getpid(), x, (void*)&x);
    }
    return 0;
}