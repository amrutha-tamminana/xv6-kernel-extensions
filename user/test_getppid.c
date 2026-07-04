#include "../kernel/types.h"
#include "user.h"
int main(){
    int ppid = getppid();
    printf("PID of parent of curr proc = %d\n", ppid);
    int child = fork();
    if(child == 0){
        int cpid_ppid = getppid();
        printf("Child's parent PID = %d\n", cpid_ppid);
        exit(0);
    } else {
        wait(0);
    }

    exit(0);
}
