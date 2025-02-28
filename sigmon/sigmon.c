#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <error.h>
#include <sys/wait.h>   /* wait() */
#include <sys/ptrace.h> /* ptrace() */


char *g_name[32] = {
    NULL,
    "SIGHUP",
    "SIGINT",
    "SIGQUIT",
    "SIGILL",
    "SIGTRAP",
    "SIGABRT",
    "SIGBUS",
    "SIGFPE",
    "SIGKILL",
    "SIGUSR1",
    "SIGSEGV",
    "SIGUSR2",
    "SIGPIPE",
    "SIGALRM",
    "SIGTERM",
    "SIGSTKFLT",
    "SIGCHLD",
    "SIGCONT",
    "SIGSTOP",
    "SIGTSTP",
    "SIGTTIN",
    "SIGTTOU",
    "SIGURG",
    "SIGXCPU",
    "SIGXFSZ",
    "SIGVTALRM",
    "SIGPROF",
    "SIGWINCH",
    "SIGIO",
    "SIGPWR",
    "SIGSYS"
};
int g_pid = 0;
int g_sig = 0;

void handler(int signo)
{
    ptrace(PTRACE_CONT, g_pid, NULL, g_sig);
    ptrace(PTRACE_DETACH, g_pid, NULL, NULL);
    printf("[sigmon] ctrl+C\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    int state = 0;
    int rc;

    if (argc < 2)
    {
        for (int i=1; i<32; i++)
        {
            printf("%2d %s\n", i, g_name[i]);
        }
        printf("\n");
        printf("Usage: sigmon PID\n\n");
        return 0;
    }

    g_pid = atoi( argv[1] );
    if (g_pid <= 0)
    {
        printf("ERR: wrong PID (%s)\n\n", argv[1]);
        return -1;
    }

    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    if (ptrace(PTRACE_ATTACH, g_pid, NULL, NULL) != 0)
    {
        perror( "ptrace (PTRACE_ATTACH)" );
        return -1;
    }

    rc = waitpid(g_pid, &state, WUNTRACED);
    if ((rc != g_pid) || !(WIFSTOPPED(state)))
    {
        printf("ERR: unexpedted waitpid result\n\n");
        ptrace(PTRACE_DETACH, g_pid, NULL, NULL);
        return -1;
    }

    printf("[sigmon] ready\n");

    for (;;)
    {
        ptrace(PTRACE_CONT, g_pid, NULL, g_sig);
        wait( &state );
        g_sig = WSTOPSIG( state );
        if (g_sig == 0) break;
        printf("pid %d got signal %d", g_pid, g_sig);
        if (g_sig < 32)
        {
            printf(" (%s)\n", g_name[g_sig]);
        }
        else
        {
            printf("\n");
        }
    }

    g_sig = 0;
    ptrace(PTRACE_CONT, g_pid, NULL, g_sig);
    ptrace(PTRACE_DETACH, g_pid, NULL, NULL);

    printf("[sigmon] exit\n");
    return 0;
}

