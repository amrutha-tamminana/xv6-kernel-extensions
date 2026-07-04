#include "kernel/types.h"
#include "user/user.h"

struct vmstats info;

int main()
{
    printf("=== Test 5: Lazy Allocation ===\n");

    char *p = sbrk(50 * 4096);
    (void)p;   // prevent unused variable warning

    getvmstats(getpid(), &info);

    printf("Faults: %d\n", info.page_faults);

    if(info.page_faults != 0){
        printf("ERROR: Not lazy allocation\n");
    } else {
        printf("Lazy allocation working\n");
    }

    exit(0);
}