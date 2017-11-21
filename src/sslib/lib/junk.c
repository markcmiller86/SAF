#include <stdio.h>

typedef struct foo_t {
    int alpha;
    unsigned long beta[2];
} foo_t;

int main()
{
    foo_t a1, b1;
    unsigned long a2[2], b2[2];

    a1.beta[0] = 11;
    a1.beta[1] = 121;
    b1.beta[0] = 15;
    b1.beta[1] = 37;

    printf("a[0]=%lu, a[1]=%lu\n", a1.beta[0], a1.beta[1]);
    a1.beta = b1.beta;
    printf("a[0]=%lu, a[1]=%lu\n", a1.beta[0], a1.beta[1]);

    a2[0] = 11;
    a2[1] = 121;
    b2[0] = 15;
    b2[1] = 37;

    printf("a[0]=%lu, a[1]=%lu\n", a2[0], a2[1]);
    a2 = b2;
    printf("a[0]=%lu, a[1]=%lu\n", a2[0], a2[1]);

    return 0;
}
