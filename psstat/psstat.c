#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <dirent.h>


// /////////////////////////////////////////////////////////////////////////////
//    Macro declarations
// /////////////////////////////////////////////////////////////////////////////

#define IS_UPPER_ALHPABET(ch) ((ch >= 'A') && (ch <= 'Z'))
#define IS_LOWER_ALHPABET(ch) ((ch >= 'a') && (ch <= 'z'))
#define IS_NUMBER(ch)         ((ch >= '0') && (ch <= '9'))
#define IS_SPACE(ch)          ((ch == ' ') || (ch == '\t') || (ch == '\n'))
#define LINE_SIZE             (1023)
#define TOKEN_SIZE            (255)


// /////////////////////////////////////////////////////////////////////////////
//    Type declarations
// /////////////////////////////////////////////////////////////////////////////

typedef struct _tTaskStat
{
    unsigned int   pid;
    char           comm[128];
    unsigned long  utime;
    unsigned long  stime;
    unsigned long  cutime;
    unsigned long  cstime;
} tTaskStat;


// /////////////////////////////////////////////////////////////////////////////
//    Variables declarations
// /////////////////////////////////////////////////////////////////////////////

tTaskStat g_taskStat;
int g_running = 1;


// /////////////////////////////////////////////////////////////////////////////
//    Functions
// /////////////////////////////////////////////////////////////////////////////

char *first_token(char *pString, char *pToken, int tsize)
{
    int i = 0;

    if (0x0 == pString[0])
    {
        /* This is a NULL line */
        pToken[0] = 0x0;
        return NULL;
    }

    /* Pass space and tab character */
    for (; *pString && IS_SPACE(*pString); pString++);

    /* Pull out the token */
    for (; *pString && !IS_SPACE(*pString) && i<tsize; pString++, i++)
    {
        *pToken++ = *pString;
    }
    *pToken = 0x0;

    return pString;
}

char *bypass_token(int num, char *pString)
{
    char  token[TOKEN_SIZE+1];
    char *pNext = pString;
    int   i;

    for (i=0; i<num; i++)
    {
        pNext = first_token(pNext, token, TOKEN_SIZE);
        if (pNext == NULL) 
        {
            break;
        }
    }

    /* Pass space and tab character */
    for (; *pNext && IS_SPACE(*pNext); pNext++);

    return pNext;
}

int task_cpu_time(
    unsigned int   pid,
    unsigned long *pUtime,
    unsigned long *pStime,
    unsigned long *pCUtime,
    unsigned long *pCStime
)
{

    FILE *pFile;
    char  pathStat[128];
    char  line[LINE_SIZE+1];
    char  token[TOKEN_SIZE+1];
    char *pNext;


    /*
    *  /proc
    *   |-- pid
    *       |-- cmdline
    *       |-- comm
    *       |-- stat
    *       |-- task
    *           |-- pid
    *               |-- stat
    *               |-- statm
    *               |-- status
    */
    sprintf(pathStat, "/proc/%u/stat", pid);

    pFile = fopen(pathStat, "r");
    if (NULL == pFile)
    {
      //printf("ERR: cannot open %s\n", pathStat);
        return -1;
    }

    if ( fgets(line, 1023, pFile) )
    {
        /* [1~13] */
        pNext = bypass_token(13, line);

        /* [14] utime */
        pNext = first_token(pNext, token, TOKEN_SIZE);
        *pUtime = strtoul(token, NULL, 10);

        /* [15] stime */
        pNext = first_token(pNext, token, TOKEN_SIZE);
        *pStime = strtoul(token, NULL, 10);

        /* [16] cutime */
        pNext = first_token(pNext, token, TOKEN_SIZE);
        *pCUtime = strtoul(token, NULL, 10);

        /* [17] cstime */
        pNext = first_token(pNext, token, TOKEN_SIZE);
        *pCStime = strtoul(token, NULL, 10);
    }

    fclose( pFile );

    return 0;
}

int process_monitor(unsigned int pid, unsigned int interval)
{
    static char timestamp[16];
    static char usagebar[128];
    struct tm *pTM;
    time_t t;

    unsigned long utime = g_taskStat.utime;
    unsigned long stime = g_taskStat.stime;
    unsigned long cutime = g_taskStat.cutime;
    unsigned long cstime = g_taskStat.cstime;
    unsigned long clocks;
    unsigned long percent;
    int i;


    sleep( interval );

    t = time( NULL );
    pTM = localtime( &t );
    sprintf(
        timestamp,
        "%02d:%02d:%02d",
        (pTM->tm_hour),
        (pTM->tm_min),
        (pTM->tm_sec)
    );

    if (task_cpu_time(
            pid,
            &(g_taskStat.utime),
            &(g_taskStat.stime),
            &(g_taskStat.cutime),
            &(g_taskStat.cstime)) != 0)
    {
      //printf("ERR: fail to get cpu time\n");
        return -1;
    }

    clocks = (g_taskStat.utime - utime) +
             (g_taskStat.stime - stime) +
             (g_taskStat.cutime - cutime) +
             (g_taskStat.cstime - cstime);
    #if 0
    percent = ((clocks * 100) / (sysconf(_SC_CLK_TCK) * interval * sysconf(_SC_NPROCESSORS_ONLN)));
    #else
    percent = ((clocks * 100) / (sysconf(_SC_CLK_TCK) * interval));
    #endif

    for (i=0; i<percent && i<100; i++)
    {
        usagebar[i] = '|';
    }
    usagebar[i] = 0x0;

    printf(
        "%s %3lu%% %s\n",
        timestamp,
        percent,
        usagebar
    );
    return 0;
}

int process_find(unsigned int pid)
{
    FILE *pFile;
    char  pathComm[64];
    char  line[LINE_SIZE+1];
    char  token[TOKEN_SIZE+1];


    #if 0
    sprintf(pathComm, "/proc/%u/comm", pid);
    #else
    sprintf(pathComm, "/proc/%u/cmdline", pid);
    #endif

    pFile = fopen(pathComm, "r");
    if (NULL == pFile)
    {
        printf("ERR: cannot open %s\n", pathComm);
        return -1;
    }

    if ( fgets(line, 1023, pFile) )
    {
        first_token(line, token, TOKEN_SIZE);
        memcpy(g_taskStat.comm, token, 127);
    }

    fclose( pFile );

    if (task_cpu_time(
            pid,
            &(g_taskStat.utime),
            &(g_taskStat.stime),
            &(g_taskStat.cutime),
            &(g_taskStat.cstime)) != 0)
    {
        printf("ERR: fail to get cpu time\n");
        return -1;
    }

    g_taskStat.pid = pid;
    return 0;
}

void terminate(int arg)
{
    g_running = 0;
}

void help(void)
{
    printf("Usage: psstat [OPTION]...\n");
    printf("\n");
    printf("  -p PID        Process ID.\n");
    printf("  -i INTERVAL   Interval in seconds.\n");
    printf("  -n LOOPS      Number of loops.\n");
    printf("  -h            Show help message.\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    unsigned int pid = 0;
    unsigned int interval = 1;
    unsigned int loops = 0;
    unsigned int count;
    int opt;


    if (argc < 2)
    {
        help();
        return 0;
    }

    opterr = 0;
    while ((opt=getopt(argc, argv, "p:i:n:h")) != -1)
    {
        switch ( opt )
        {
            case 'p':
                pid = strtoul(optarg, NULL, 10);
                break;
            case 'i':
                interval = atoi( optarg );
                if (0 == interval) interval = 0;
                break;
            case 'n':
                loops = atoi( optarg );
                break;
            case 'h':
            default:
                help();
                return 0;
        }
    }

    if (0 == pid)
    {
        help();
        return 0;
    }

    signal(SIGINT,  terminate);
    signal(SIGKILL, terminate);
    signal(SIGTERM, terminate);

    memset(&g_taskStat, 0, sizeof( tTaskStat ));
    count = 0;

    if (process_find( pid ) != 0)
    {
        printf("ERR: cannot find task[%u]\n\n", pid);
        return -1;
    }

    printf("%s (%u):\n", g_taskStat.comm, g_taskStat.pid);

    do
    {
        if (process_monitor(pid, interval) != 0) break;
        if (++count == loops) break;
    } while ( g_running );

    return 0;
}

