#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int main() {
  int before=getsyscount();
  getpid();
  getpid();
  getpid();
  int after=getsyscount();
  printf("syscount before = %d\n",before);
  printf("syscount after = %d\n",after);
  printf("difference = %d\n",after-before);
  exit(0);
}