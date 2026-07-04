#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
int main() {
    int pid1=fork();
    if(pid1==0){
        pause(50);
        exit(0);
  }
    int pid2=fork();
    if(pid2==0){
        pause(50);
        exit(0);
  }
    pause(1);
    int n=getnumchild();
    printf("Number of children = %d\n",n);
    wait(0);
    wait(0);
    exit(0);
}
