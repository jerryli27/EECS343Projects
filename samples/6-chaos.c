#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

int main(void) {
    int pid = fork();
    int status;
    if (pid == 0) {
        printf("Child process started...\n");
        sleep(4);

        printf("Child process phase 2\n");
        sleep(6);

        printf("Child process finished!\n");
    } else {
        printf("I am the parent of PID: %d\n", pid);
        sleep(2);

        printf("Parent stopping child\n");
        kill(pid, SIGTSTP);
        for (int count = 5; count > 0; count--) {
            sleep(1);
            printf("Sleep count down... %d\n", count);
        }
        kill(pid, SIGCONT);

        printf("Parent waiting for child\n");
        waitpid(pid, &status, 0);
    }
}
