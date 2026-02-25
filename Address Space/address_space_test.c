#include <stdio.h>
#include <stdlib.h>

int main() {

    printf("===== Address Space Test =====\n");

    printf("Code  address : %p\n", (void*)main);

    void *heap = malloc(1);
    printf("Heap  address : %p\n", heap);

    int x = 10;
    printf("Stack address : %p\n", (void*)&x);

    return 0;
}