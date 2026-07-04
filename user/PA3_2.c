// Credit: Akshat
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    struct vmstats st;
    int pgsz = 4096;
    int max_pages = 250;
    char *base = sbrklazy(0);
    
    printf("\n--- Test 2: Eviction, Verification, and Swap-In ---\n");
    printf("Allocating memory until evictions trigger...\n");

    int pages_allocated = 0;
    for(int i = 0; i < max_pages; i++) {
        char *mem = sbrklazy(pgsz);
        if(mem == (char*)-1) {
            printf("sbrk failed at page %d\n", i);
            break;
        }
        
        // Touch the page to force allocation and write predictable data
        mem[0] = (char)(i % 256); 
        pages_allocated++;

        // Check if we've successfully triggered evictions yet
        getvmstats(getpid(), &st);
        if(st.pages_evicted > 150) {
            printf("Successfully triggered %d evictions!\n", st.pages_evicted);
            break;
        }
    }

    printf("Reading back memory to verify and trigger swap-ins...\n");
    int corruption_errors = 0;
    char *curr = base;
    for(int i = 0; i < pages_allocated; i++) {
        // Read the data back. If it was swapped, this triggers a swap-in!
        if(curr[0] != (char)(i % 256)) {
            corruption_errors++;
            printf("page number %d expected : %c got : %c\n",i,(char)(i%256),curr[0]);
        }
        curr += pgsz;
    }

    getvmstats(getpid(), &st);
    printf("Verification complete. Errors: %d\n", corruption_errors);
    printf("Pages swapped in: %d\n", st.pages_swapped_in);
    
    if(corruption_errors == 0 && st.pages_swapped_in > 0) {
        printf("Test 2 PASSED!\n\n");
    } else {
        printf("Test 2 FAILED!\n\n");
    }
    
    exit(0);
}