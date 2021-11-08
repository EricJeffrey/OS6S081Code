#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  int pid = fork();
  if (pid < 0)
    exit(1);
  if (pid == 0){
    printf("child running, pid:%d\n", pid);
    exit(0);
  }
  else {
    printf("father waiting, pid:%d\n", pid);
    wait(0);
    exit(0);
  }
}
