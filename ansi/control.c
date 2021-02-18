#include <stdio.h>

char BEL = '\a';
char BS  = '\b';
char ESC = '\e';
char FF  = '\f';
char LF  = '\n';
char CR  = '\r';
char HT  = '\t';
char VT  = '\v';

int main(void)
{
    printf("\\a  0x%02X  BELL             [%c]\n", BEL, BEL);
    printf("\\b  0x%02X  BACKSPACE        [%c]\n", BS, BS);
    printf("\\e  0x%02X  ESCAPE           \n", ESC);
    printf("\\n  0x%02X  LINE FEED        [%c]\n", LF, LF);
    printf("\\r  0x%02X  CARRIAGE RETURN  [%c]\n", CR, CR);
    printf("\\t  0x%02X  HORIZONTAL TAB   [%c]\n", HT, HT);
    printf("\\u  0x%02X  VERTICAL TAB     [%c]\n", VT, VT);

    printf("\n");
    return 0;
}

