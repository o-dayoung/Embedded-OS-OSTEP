#include <stdio.h>

void test(int n) {
    int x;
    printf("Stack address : %p\n", &x);

    if (n > 0)
        test(n - 1);
}

int main() {
    test(5);
    return 0;
}