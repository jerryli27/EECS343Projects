#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int pid = fork();
    int status;
    if (pid == 0) {
        char *argv[] = { "/bin/ps", 0};
        execve("/bin/ps", argv, NULL);
    } else {
        printf("I am the parent of PID: %d\n", pid);
        waitpid(pid, &status, 0);
        printf("Parent done! Child status: %d\n", status);

    }
}
