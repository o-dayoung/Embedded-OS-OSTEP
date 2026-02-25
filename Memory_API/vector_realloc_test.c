#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *data;
    int size;
    int capacity;
} Vector;

void init(Vector *v) {
    v->capacity = 2;
    v->size = 0;
    v->data = malloc(v->capacity * sizeof(int));
}

void push(Vector *v, int value) {
    if (v->size == v->capacity) {
        v->capacity *= 2;
        v->data = realloc(v->data, v->capacity * sizeof(int));
    }
    v->data[v->size++] = value;
}

void destroy(Vector *v) {
    free(v->data);
}

int main() {
    Vector v;
    init(&v);

    for(int i=0;i<10;i++)
        push(&v, i);

    destroy(&v);
}