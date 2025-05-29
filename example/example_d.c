#define NULL 0

int *foo() {}
long bar() {}

int main() {
    int x, y = (int)1;
    int *a = &x, **b, ***c = NULL;
    const int *d = &y, e = 5;
    static volatile char *p, q = 'z', **r = &p;
    int *****x = NULL, y = 10, *z = &y;

    int **arr;
    x++;
    arr[5]++;
    foo()[2]--;
    int **ptr;

    *p++;
    *--p;
    a[1 + 2]++;
    (*ptr)[5];

    arr[1]++;
    *arr[3]--;
    foo()[bar()]++;
    int value = arr[x++] + y[--z];
    ((int *)value)[x]--;
    
    unsigned long long int ***x[1][1];

    return 0;
}