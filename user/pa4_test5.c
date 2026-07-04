#include "kernel/types.h"
#include "user/user.h"

int main() {
    struct vmstats st;

    setdisksched(0);

    for(int i = 0; i < 150; i++) {
        char *m = sbrklazy(4096);
        if(m == (char*)-1) break;
        m[0] = i;
    }

    getvmstats(getpid(), &st);
    int fcfs = st.pages_swapped_out;

    setdisksched(1);

    for(int i = 0; i < 150; i++) {
        char *m = sbrklazy(4096);
        if(m == (char*)-1) break;
        m[0] = i;
    }

    getvmstats(getpid(), &st);
    int sstf = st.pages_swapped_out;
    printf("ACTUAL: FCFS swaps = %d SSTF swaps = %d\n", fcfs, sstf);

    exit(0);
}