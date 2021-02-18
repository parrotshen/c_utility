#include <stdio.h>

#define ANSI_FG_BLACK    "\033[1;30m"
#define ANSI_FG_RED      "\033[1;31m"
#define ANSI_FG_GREEN    "\033[1;32m"
#define ANSI_FG_YELLOW   "\033[1;33m"
#define ANSI_FG_BLUE     "\033[1;34m"
#define ANSI_FG_MAGENTA  "\033[1;35m"
#define ANSI_FG_CYAN     "\033[1;36m"
#define ANSI_FG_WHITE    "\033[1;37m"

#define ANSI_BG_BLACK    "\033[37;40m"
#define ANSI_BG_RED      "\033[30;41m"
#define ANSI_BG_GREEN    "\033[30;42m"
#define ANSI_BG_YELLOW   "\033[30;43m"
#define ANSI_BG_BLUE     "\033[30;44m"
#define ANSI_BG_MAGENTA  "\033[30;45m"
#define ANSI_BG_CYAN     "\033[30;46m"
#define ANSI_BG_WHITE    "\033[30;47m"

#define ANSI_OFF         "\033[0m"

int main(int argc, char *argv[])
{
    printf(
        "BLACK   :  %sFOREGROUND 30%s  %sBACKGROUND 40%s\n",
        ANSI_FG_BLACK,
        ANSI_OFF,
        ANSI_BG_BLACK,
        ANSI_OFF
    );
    printf(
        "RED     :  %sFOREGROUND 31%s  %sBACKGROUND 41%s\n",
        ANSI_FG_RED,
        ANSI_OFF,
        ANSI_BG_RED,
        ANSI_OFF
    );
    printf(
        "GREEN   :  %sFOREGROUND 32%s  %sBACKGROUND 42%s\n",
        ANSI_FG_GREEN,
        ANSI_OFF,
        ANSI_BG_GREEN,
        ANSI_OFF
    );
    printf(
        "YELLOW  :  %sFOREGROUND 33%s  %sBACKGROUND 43%s\n",
        ANSI_FG_YELLOW,
        ANSI_OFF,
        ANSI_BG_YELLOW,
        ANSI_OFF
    );
    printf(
        "BLUE    :  %sFOREGROUND 34%s  %sBACKGROUND 44%s\n",
        ANSI_FG_BLUE,
        ANSI_OFF,
        ANSI_BG_BLUE,
        ANSI_OFF
    );
    printf(
        "MAGENTA :  %sFOREGROUND 35%s  %sBACKGROUND 45%s\n",
        ANSI_FG_MAGENTA,
        ANSI_OFF,
        ANSI_BG_MAGENTA,
        ANSI_OFF
    );
    printf(
        "CYAN    :  %sFOREGROUND 36%s  %sBACKGROUND 46%s\n",
        ANSI_FG_CYAN,
        ANSI_OFF,
        ANSI_BG_CYAN,
        ANSI_OFF
    );
    printf(
        "WHITE   :  %sFOREGROUND 37%s  %sBACKGROUND 47%s\n",
        ANSI_FG_WHITE,
        ANSI_OFF,
        ANSI_BG_WHITE,
        ANSI_OFF
    );
    printf("\n");

    return 0;
}
