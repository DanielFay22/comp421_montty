

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

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

    char *buf = (char *)(malloc(sizeof(char) * (buflen + 1)));

    ReadTerminal(term, buf, buflen);
    buf[buflen] = '\n';

    printf("Read string: %10s\n", buf);

    WriteTerminal(term, buf, buflen + 1);
}


int main(void) {

    InitTerminalDriver();
    InitTerminal(1);

    ThreadCreate(writer, NULL);
    ThreadCreate(reader, NULL);
    ThreadCreate(reader, NULL);
    ThreadCreate(reader, NULL);

    ThreadWaitAll();

    sleep(10);

    exit(0);

    return 0;
}