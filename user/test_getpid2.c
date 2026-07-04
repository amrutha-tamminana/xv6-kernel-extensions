#include "../kernel/types.h"
#include "user.h"
int main(){
    int pid=getpid2();
    printf("pid = %d\n",pid);
    exit(0);
}