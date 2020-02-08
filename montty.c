

#include "hardware.h"
#include "terminals.h"
#include "threads.h"


int input_char(int term);
int write_chars(int term, char *buf, int buflen);
int read_chars(int term, char *buf, int buflen);
void set_data_register_ready(int term);
int get_stats(struct termstat *cpy_stats);
void init_monitor();


void TransmitInterrupt(int term) {
    set_data_register_ready(term);
}

void ReceiveInterrupt(int term) {
    input_char(term);
}

int WriteTerminal(int term, char *buf, int buflen) {
    return write_chars(term, buf, buflen);
}

int ReadTerminal(int term, char *buf, int buflen) {
    return read_chars(term, buf, buflen);
}

int InitTerminal(int term) {
    return InitHardware(term);
}

int TerminalDriverStatistics(struct termstat *stats) {
    return get_stats(stats);
}

int InitTerminalDriver(void) {
    init_monitor();
}