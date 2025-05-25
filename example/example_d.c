#define NULL 0

int main() {
    int x, y = (int)1;
    int *a = &x, **b, ***c = NULL;
    const int *d = &y, e = 5;
    static volatile char *p, q = 'z', **r = &p;
    int *****x = NULL, y = 10, *z = &y;

    x++;
    arr[5]++;
    foo()[2]--;

    *p++;
    *--p;
    a[1 + 2]++;
    (*ptr)[5];

    arr[1]++;
    *ptr[3]--;
    foo()[bar()]++;
    value = arr[x++] + y[--z];
    ((int *)data)[index]--;

    return 0;
}