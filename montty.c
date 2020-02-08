
#include <stdlib.h>

#include "hardware.h"
#include "terminals.h"
#include "threads.h"

#include "monitor.c"



void ReceiveInterrupt(int term);
void TransmitInterrupt(int term);

int WriteTerminal(int term, char *buf, int buflen);
int ReadTerminal(int term, char *buf, int buflen);

int InitTerminal(int term);
int TerminalDriverStatistics(struct termstat *stats);
int InitTerminalDriver(void);


void user_thread(int *term) {
    InitTerminal(*term);

    char *buf = (char *) (malloc(sizeof(char) * BUF_LEN));
    int read;

    while (1) {
        read = ReadTerminal(*term, buf, BUF_LEN);
        WriteTerminal(*term, buf, read);
    }
}


int main(void) {
    int i;

    InitTerminalDriver();



    for (i = 0; i < NUM_TERMINALS; i ++)
        ThreadCreate(user_thread, &i);

    ThreadWaitAll();

    return 0;
}