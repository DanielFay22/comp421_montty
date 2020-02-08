

#include "hardware.h"
#include "terminals.h"
#include "threads.h"

#include <stdio.h>


#define BUF_LEN 4096

static void flush_output(int term);


//static cond_id_t out_full[NUM_TERMINALS];
static cond_id_t inp_empty[NUM_TERMINALS];
//static cond_id_t output_ready[NUM_TERMINALS];

static cond_id_t data_register_ready[NUM_TERMINALS];
//static cond_id_t data_register_read[NUM_TERMINALS];

static int input_chars[NUM_TERMINALS] = {0};
static int output_chars[NUM_TERMINALS] = {0};
static int echo_chars[NUM_TERMINALS] = {0};

static int input_buffer[NUM_TERMINALS][BUF_LEN];
static int output_buffer[NUM_TERMINALS][BUF_LEN];
static int echo_buffer[NUM_TERMINALS][BUF_LEN];

static int input_write_pos[NUM_TERMINALS] = {0};
static int output_write_pos[NUM_TERMINALS] = {0};
static int echo_write_pos[NUM_TERMINALS] = {0};

static int input_read_pos[NUM_TERMINALS] = {0};
static int output_read_pos[NUM_TERMINALS] = {0};
static int echo_read_pos[NUM_TERMINALS] = {0};

static int is_writing[NUM_TERMINALS] = {0};
static cond_id_t can_write[NUM_TERMINALS];

static struct termstat stats[NUM_TERMINALS] = {{0,0,0,0}};


extern void ReceiveInterrupt(int term) {
    Declare_Monitor_Entry_Procedure();

    char c = ReadDataRegister(term);

    printf("Received character %c\n", c);

    // If either buffer is full, ignore the event.
    if (input_chars[term] == BUF_LEN || echo_chars[term] == BUF_LEN)
        return;

    input_buffer[term][input_write_pos[term]] = c;
    echo_buffer[term][echo_write_pos[term]] = c;

    input_write_pos[term] = (input_write_pos[term] + 1) % BUF_LEN;
    echo_write_pos[term] = (echo_write_pos[term] + 1) % BUF_LEN;

    ++input_chars[term];
    ++echo_chars[term];

    stats[term].tty_in += 1;

    flush_output(term);

    // Signal input buffer is non-empty.
    CondSignal(inp_empty[term]);
}

extern void TransmitInterrupt(int term) {
    Declare_Monitor_Entry_Procedure();

    printf("Transmit interrupt received\n");

    CondSignal(data_register_ready[term]);
}

static void flush_output(int term) {
    printf("flush_output\n");

    if (output_chars[term] + echo_chars[term] > 0) {

        if (echo_chars[term] > 0) {
            WriteDataRegister(term, echo_buffer[term][echo_read_pos[term]]);
            echo_read_pos[term] = (echo_read_pos[term] + 1) % BUF_LEN;
            echo_chars[term]--;
        } else {
            WriteDataRegister(term, output_buffer[term][output_read_pos[term]]);
            output_read_pos[term] = (output_read_pos[term] + 1) % BUF_LEN;
            output_chars[term]--;
        }

        printf("waiting on register\n");
        CondWait(data_register_ready[term]);

        stats[term].tty_out += 1;

    }
}

extern int WriteTerminal(int term, char *buf, int buflen) {
    Declare_Monitor_Entry_Procedure();

    while (is_writing[term] > 0)
        CondWait(can_write[term]);
    is_writing[term] = 1;

    int i;
    for (i = 0; i < buflen; i++) {

        while (output_chars[term] == BUF_LEN) {
            printf("Flushing Output\n");
            flush_output(term);
        }

        printf("Writing character\n");
        output_buffer[term][output_write_pos[term]] = buf[i];

        output_write_pos[term] = (output_write_pos[term] + 1) % BUF_LEN;
        ++output_chars[term];

        flush_output(term);
    }

    printf("Finished WriteTerminal\n");
    flush_output(term);

    stats[term].user_in += i;

    is_writing[term] = 0;
    CondSignal(can_write[term]);

    return i;

}

extern int ReadTerminal(int term, char *buf, int buflen) {
    Declare_Monitor_Entry_Procedure();

    int i = 0;
    char c = ' ';

    while (i < buflen && c != '\n') {
        while (input_chars[term] == 0)
            CondWait(inp_empty[term]);

        c = (buf[i++] = input_buffer[term][input_read_pos[term]]);
        input_read_pos[term] = (input_read_pos[term] + 1) % BUF_LEN;
        input_chars[term]--;

    }

    stats[term].user_out += i;

    return i;
}

extern int InitTerminal(int term) {
    Declare_Monitor_Entry_Procedure();

//    out_full[term] = CondCreate();
//    inp_empty[term] = CondCreate();
//    data_register_ready[term] = CondCreate();

    CondSignal(data_register_ready[term]);

    return InitHardware(term);;
}

extern int TerminalDriverStatistics(struct termstat *cpy_stats) {
    Declare_Monitor_Entry_Procedure();

    int i;

    for (i = 0; i < NUM_TERMINALS; i ++)
        cpy_stats[i] = stats[i];

    return 0;
}

extern int InitTerminalDriver() {
    Declare_Monitor_Entry_Procedure();

    int i;

    for (i = 0; i < NUM_TERMINALS; i++) {
        inp_empty[i] = CondCreate();
        data_register_ready[i] = CondCreate();
        can_write[i] = CondCreate();
    }

    return 0;
}