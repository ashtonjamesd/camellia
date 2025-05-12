#include <stdio.h>

#define LEFT 10
#define RIGHT 10 + 2
#undef LEFT

#define LEFT

void main() {
    int x = LEFT + RIGHT;

}