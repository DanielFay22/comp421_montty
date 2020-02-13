

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "terminals.h"
#include "threads.h"

#define BUF_SIZE 16384



void user_thread(void *arg) {
    int term = 0, i = 0;

    InitTerminal(term);

    char *buf = (char *) (malloc(sizeof(char) * BUF_SIZE));
    int read;

    while (i < 10) {
        read = ReadTerminal(term, buf, BUF_SIZE);
        buf[read] = '\0';
        printf("Read: %s", buf);
        WriteTerminal(term, buf, read);
        ++i;
    }
}


int main(void) {

    InitTerminalDriver();

    ThreadCreate(user_thread, NULL);

    ThreadWaitAll();

    sleep(5);

    struct termstat *stats =
        (struct termstat *)(malloc(sizeof(struct termstat) * 4));

    TerminalDriverStatistics(stats);

    printf("tty_in: %d\ttty_out: %d\tuser_in: %d\tuser_out: %d\n",
        stats->tty_in, stats->tty_out,
        stats->user_in, stats->user_out
        );

    sleep(10);

    exit(0);

    return 0;
}