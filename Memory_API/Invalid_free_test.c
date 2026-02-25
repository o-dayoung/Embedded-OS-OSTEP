int *data = malloc(100 * sizeof(int));
free(&data[50]);   // 잘못된 포인터