#include "kernel/types.h"
#include "user/user.h"

int main() {
    struct vmstats st;
    int pgsz = 4096;

    int total = 300;
    char *base = sbrklazy(0);

    for(int i = 0; i < total; i++) {
        char *m = sbrklazy(pgsz);
        if(m == (char*)-1) break;
        m[0] = i;
    }

    getvmstats(getpid(), &st);

    printf("EXPECTED: pages_evicted > 0\n");
    printf("ACTUAL: pages_evicted = %d\n", st.pages_evicted);

    printf("EXPECTED: pages_swapped_out > 0\n");
    printf("ACTUAL: pages_swapped_out = %d\n", st.pages_swapped_out);

    char *p = base;

    for(int round = 0; round < 5; round++) {
        p = base;
        for(int i = 0; i < total; i++) {
            if(p[i] == -1) printf("a\n");
            p += pgsz;
        }
    }

    getvmstats(getpid(), &st);
    
    exit(0);
}