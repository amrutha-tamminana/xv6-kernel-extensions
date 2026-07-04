#include "kernel/types.h"
#include "user/user.h"

int main() {
    setdisksched(1);

    int pgsz = 4096;
    char *pages[20];

    for(int i = 0; i < 20; i++) {
        pages[i] = sbrklazy(pgsz);
        if(pages[i] == (char*)-1) exit(0);
        pages[i][0] = i;
    }

    printf("EXPECTED: SSTF reorders requests\n");

    int order[] = {0, 19, 1, 18, 2, 17, 3, 16, 4, 15};

    for(int i = 0; i < 10; i++) {
        if(pages[order[i]][0] == -1) printf("a\n");
    }

    exit(0);
}