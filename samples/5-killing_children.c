#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

int main(void)
{
    int pid = fork();
    int status;
    if (pid == 0) {
        printf("Child process started...\n");
        sleep(10);
        printf("Child process finished!\n");
    } else {
        printf("I am the parent of PID: %d\n", pid);
        sleep(2);
        printf("Parent process that won't wait. Sending kill signal to children\n");
        kill(pid, SIGINT);
    }
}
