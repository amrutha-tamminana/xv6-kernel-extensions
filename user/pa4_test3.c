#include "kernel/types.h"
#include "user/user.h"

int main() {
    setdisksched(0);

    int pgsz = 4096;

    char *pages[50];

    for(int i = 0; i < 50; i++) {
        pages[i] = sbrklazy(pgsz);
        if(pages[i] == (char*)-1) exit(0);
        pages[i][0] = i;
    }

    printf("EXPECTED: FCFS order\n");

    for(int i = 0; i < 50; i++) {
        if(pages[i][0] == -1) printf("a\n");
    }

    exit(0);
}