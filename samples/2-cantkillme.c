#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sig_handler(int signo)
{
    if (signo == SIGINT)
        printf("SIGINT can't kill me!!\n");
}

int main(void)
{
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("Error registring signal handler\n");
    while(1) 
        sleep(1);
    return 0;
}
