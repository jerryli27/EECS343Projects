#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

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
        //waitpid(pid, &status, 0);
        printf("Parent done! Child status: %d\n", status);

    }
}
