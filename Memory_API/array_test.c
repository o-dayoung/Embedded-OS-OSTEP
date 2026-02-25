#include <stdlib.h>

int main() {
    int *data = malloc(100 * sizeof(int));
    data[100] = 0;   // 범위 초과
    free(data);
}