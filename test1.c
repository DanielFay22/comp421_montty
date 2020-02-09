

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

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
        buf[read] = '\0';
        printf("Read: %s", buf);
        WriteTerminal(term, buf, read);
    }
}


int main(void) {

    InitTerminalDriver();

//    ThreadCreate(user_thread, NULL);
//
//    ThreadWaitAll();

    struct termstat *stats =
        (struct termstat *)(malloc(sizeof(struct termstat) * 4));

    printf("tty_in: %d\ttty_out: %d\tuser_in: %d\tuser_out: %d\n",
        stats[0].tty_in, stats[0].tty_out,
        stats[0].user_in, stats[0].user_out
        );

    sleep(10);

    exit(0);

    return 0;
}