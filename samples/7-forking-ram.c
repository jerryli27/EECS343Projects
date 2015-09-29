#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int pid = fork();
    int x = 42;
    if (pid == 0) {
        // Toggle sleep to stop child
        // sleep(10);
        printf("I am the child process\n");
        x = 13;
        printf("Child set x to: %d\n", x);
    } else {
        // Toggle sleep to stop parent
        sleep(10);
        printf("I am the parent of PID: %d\n", pid);
        printf("Parent's x: %d\n", x);
    }

}
