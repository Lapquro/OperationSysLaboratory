#define main lab_main
#include "main.c"
#undef main

int errors = 0;

void print_array(const char *title, const int *arr, int len)
{
    printf("  %s", title);
    for (int i = 0; i < len; i++)
        printf(" %d", arr[i]);
    printf("\n");
}

void run_test(const char *name, const int *src, int n, int t, int show)
{
    size = 2;
    while (size < n)
        size *= 2;
    threads = t;

    int *seq = malloc(sizeof(int) * size);
    a = malloc(sizeof(int) * size);

    long long sum_before = 0;
    for (int i = 0; i < size; i++) {
        a[i] = i < n ? src[i] : INT_MAX;
        if (i < n)
            sum_before += src[i];
    }
    sort_sequential();
    memcpy(seq, a, sizeof(int) * size);

    for (int i = 0; i < size; i++)
        a[i] = i < n ? src[i] : INT_MAX;
    sort_parallel();

    int ok = 1;
    long long sum_after = 0;
    for (int i = 0; i < n; i++)
        sum_after += a[i];
    for (int i = 1; i < size; i++)
        if (a[i - 1] > a[i])
            ok = 0;
    if (sum_before != sum_after || memcmp(a, seq, sizeof(int) * size) != 0)
        ok = 0;

    printf("%s, n = %d, threads = %d: %s\n", name, n, t, ok ? "OK" : "ERROR");
    if (show) {
        print_array("before:", src, n);
        print_array("after: ", a, n);
    }
    if (!ok)
        errors++;
    free(seq);
    free(a);
}

int main(void)
{
    int t1[] = {5, 2, 7, 1, 8, 3, 6, 4};
    int t2[] = {11, 4, 15, 2, 9, 13, 6, 1, 16, 8, 3, 12, 7, 14, 5, 10};
    int t3[] = {1, 2, 3, 4, 5, 6, 7, 8};
    int t4[] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    int t5[] = {3, 1, 3, 1, 2, 2, 3, 1};
    int t6[] = {-5, 3, 0, -1, 7, -8, 2, 4};
    int t7[] = {4, 3, 2, 1};

    run_test("test 1, random", t1, 8, 2, 1);
    run_test("test 2, random", t2, 16, 4, 1);
    run_test("test 3, already sorted", t3, 8, 3, 1);
    run_test("test 4, reverse, size not power of 2", t4, 10, 4, 1);
    run_test("test 5, duplicates", t5, 8, 2, 1);
    run_test("test 6, negative numbers", t6, 8, 4, 1);
    run_test("test 7, more threads than elements", t7, 4, 16, 1);

    int big_n = 100000;
    int *big = malloc(sizeof(int) * big_n);
    srand(7);
    for (int i = 0; i < big_n; i++)
        big[i] = rand();
    run_test("test 8, big random array", big, big_n, 16, 0);
    run_test("test 9, big random array", big, big_n, 1024, 0);
    free(big);

    if (errors == 0)
        printf("all tests passed\n");
    else
        printf("failed tests: %d\n", errors);
    return errors != 0;
}
