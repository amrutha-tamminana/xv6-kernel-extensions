#include "kernel/types.h"
#include "user/user.h"
int main(void) {
    int pid=fork();
    if(pid<0){
        printf("fork failed\n");
        exit(1);
    }
    if(pid==0){
        getpid2();
        getppid();
        getnumchild();
        exit(0);
    }
    pause(10);
    int total=getchildsyscount(pid);
    if(total<0){
        printf("Invalid PID\n");
    } else {
        printf("Total syscalls made by child PID %d: %d\n",pid,total);
    }
    wait(0);
    exit(0);
}
