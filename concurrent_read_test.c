

#include <stdlib.h>
#include <unistd.h>

#include "terminals.h"
#include "threads.h"

#define BUF_SIZE 16384



void writer(void *arg) {
    int term = 1;

    char buf[] = "aaaaaaaaaabbbbbbbbbbccccccccccddddddddddeeeeeeeeee";

    WriteTerminal(term, buf, 50);

    sleep(5);

    WriteTerminal(term, "\n", 1);
}

void reader(void *arg) {
    int term = 1;
    int buflen = 10;

    char *buf = (char *)(malloc(sizeof(char) * buflen));

    ReadTerminal(term, buf, buflen);

    WriteTerminal(term, buf, buflen);
}


int main(void) {
    int i = 1;

    InitTerminalDriver();
    InitTerminal(1);

    ThreadCreate(writer, NULL);
    ThreadCreate(reader, NULL);
    ThreadCreate(reader, NULL);
    ThreadCreate(reader, NULL);

    ThreadWaitAll();

    sleep(10);

    exit(0);


//    for (i = 0; i < NUM_TERMINALS; i ++)
//        ThreadCreate(user_thread, &i);
//
//    ThreadWaitAll();

    return 0;
}