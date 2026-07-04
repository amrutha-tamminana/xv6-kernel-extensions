#include "kernel/types.h"
#include "user/user.h"

int main() {
    int pgsz = 4096;
    int pages = 300;
    char *base = sbrklazy(0);

    for(int i = 0; i < pages; i++) {
        char *m = sbrklazy(pgsz);
        if(m == (char*)-1) break;
        m[0] = (char)(i % 128);
    }

    int errors = 0;
    char *curr = base;

    for(int i = 0; i < pages; i++) {
        if(curr[0] != (char)(i % 128)) {
            errors++;
            printf("Mismatch at %d expected %d got %d\n", i, (char)(i%128), curr[0]);
        }
        curr += pgsz;
    }

    printf("EXPECTED: errors = 0\n");
    printf("ACTUAL: errors = %d\n", errors);

    exit(0);
}