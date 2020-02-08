

#include <stdlib.h>
#include <unistd.h>

#include "terminals.h"
#include "threads.h"

#define BUF_SIZE 16384


//void ReceiveInterrupt(int term);
//void TransmitInterrupt(int term);
//
//int WriteTerminal(int term, char *buf, int buflen);
//int ReadTerminal(int term, char *buf, int buflen);
//
//int InitTerminal(int term);
//int TerminalDriverStatistics(struct termstat *stats);
//int InitTerminalDriver(void);


void user_thread(void *arg) {
    int term = 1;

    InitTerminal(term);

    char *buf = (char *) (malloc(sizeof(char) * BUF_SIZE));
    int read;

    while (1) {
        read = ReadTerminal(term, buf, BUF_SIZE);
        WriteTerminal(term, buf, read);
    }
}


int main(void) {
    int i = 1;

    InitTerminalDriver();
//    InitTerminal(1);

    ThreadCreate(user_thread, NULL);

    sleep(100);

    exit(0);


//    for (i = 0; i < NUM_TERMINALS; i ++)
//        ThreadCreate(user_thread, &i);
//
//    ThreadWaitAll();

    return 0;
}