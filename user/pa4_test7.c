#include "kernel/types.h"
#include "user/user.h"

int main() {
    int pgsz = 4096;
    char *base = sbrklazy(0);

    for(int i = 0; i < 100; i++) {
        char *m = sbrklazy(pgsz);
        if(m == (char*)-1) break;
        m[0] = (char)(i % 100);
    }

    int errors = 0;
    char *curr = base;

    for(int i = 0; i < 100; i++) {
        if(curr[0] != (char)(i % 100)) {
            errors++;
        }
        curr += pgsz;
    }

    printf("EXPECTED: errors = 0 due to mirroring\n");
    printf("ACTUAL: errors = %d\n", errors);

    exit(0);
}