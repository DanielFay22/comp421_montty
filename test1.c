

#include <stdlib.h>
#include <unistd.h>

#include "terminals.h"
#include "threads.h"

#define BUF_SIZE 16384



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

    InitTerminalDriver();

    ThreadCreate(user_thread, NULL);

    ThreadWaitAll();

    exit(0);

    return 0;
}