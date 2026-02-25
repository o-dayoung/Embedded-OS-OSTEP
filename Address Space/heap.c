#include <stdio.h>
#include <stdlib.h>

int main() {

    void *a = malloc(10);
    void *b = malloc(10);
    void *c = malloc(10);

    printf("a : %p\n", a);
    printf("b : %p\n", b);
    printf("c : %p\n", c);

    return 0;
}