#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int loop = 120;
    int i;

    if (argc > 1) loop = atoi( argv[1] );

    pid = getpid();
    printf("%d: begin\n", pid);
    for (i=0; i<loop; i++)
    {
        sleep(1);
        printf("%d: %d\n", pid, (i+1));
    }
    printf("%d: end\n", pid);

    return 0;
}
