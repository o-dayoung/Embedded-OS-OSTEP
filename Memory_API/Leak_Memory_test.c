#include <stdlib.h>

int main() {
    int *p = malloc(sizeof(int));
    *p = 100;
    return 0;   // free 안함
}