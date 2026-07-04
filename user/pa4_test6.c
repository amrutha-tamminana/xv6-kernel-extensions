#include "kernel/types.h"
#include "user/user.h"

int main() {
    int pgsz = 4096;
    int total = 200;

    char *pages[200];

    for(int i = 0; i < total; i++) {
        pages[i] = sbrklazy(pgsz);
        if(pages[i] == (char*)-1) break;
        pages[i][0] = i;
    }

    printf("EXPECTED: RAID WRITE during eviction and RAID READ during access\n");

    int errors = 0;

    for(int i = 0; i < total; i++) {
        if(pages[i][0] != (char)i) {
            errors++;
        }
    }

    printf("ACTUAL: data errors = %d\n", errors);

    exit(0);
}