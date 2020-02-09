

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "terminals.h"
#include "threads.h"

#define BUF_SIZE 16384



void writer(void *arg) {
    int term = *(int *)(arg);

    char buf[] = "abcdefghijklmnopqrstuvwxyz";

    WriteTerminal(term, buf, sizeof(buf));

    sleep(5);

    WriteTerminal(term, "\n", 1);
}

void reader(void *arg) {
    int term = *(int *)(arg);
    int buflen = 10;

    char *buf = (char *)(malloc(sizeof(char) * (buflen + 1)));

    ReadTerminal(term, buf, buflen);
    buf[buflen] = '\n';

    printf("Read string: %10s\n", buf);

    WriteTerminal(term, buf, buflen + 1);
}


int main(void) {
    int *term_1, *term_2;
    InitTerminalDriver();
    InitTerminal(1);
    InitTerminal(2);

    term_1 = (int *)malloc(sizeof(int));
    term_2 = (int *)malloc(sizeof(int));

    *term_1 = 1;
    *term_2 = 2;

    ThreadCreate(writer, (void *)term_1);
    ThreadCreate(writer, (void *)term_2);
    ThreadCreate(reader, (void *)term_1);
    ThreadCreate(reader, (void *)term_2);

    ThreadWaitAll();

    sleep(10);

    exit(0);

    return 0;
}