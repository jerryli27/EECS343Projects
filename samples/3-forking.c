#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int pid = fork();
    if (pid == 0) {
        // Toggle sleep to stop child
        // sleep(10);
        printf("I am the child process\n");
    } else {
        // Toggle sleep to stop parent
        // sleep(10);
        printf("I am the parent of PID: %d\n", pid);
    }

}
