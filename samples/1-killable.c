#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("1 You can kill me with CTRL-C\n");
    
    while(1) 
        sleep(1);
    return 0;
    
}
