

#include "hardware.h"
#include "terminals.h"
#include "threads.h"

#include <stdio.h>


#define BUF_LEN 4096

// Function definitions.
static void flush_output(int term);
static void echo(int term, char *buf, int buflen);

static cond_id_t inp_empty[NUM_TERMINALS];

static int wait_for_data_reg[NUM_TERMINALS] = {0};
static cond_id_t data_register_ready[NUM_TERMINALS];

// Keep track of number of characters in buffers.
static int input_chars[NUM_TERMINALS] = {0};
static int output_chars[NUM_TERMINALS] = {0};
static int echo_chars[NUM_TERMINALS] = {0};
static int line_chars[NUM_TERMINALS] = {0};

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

static int is_reading[NUM_TERMINALS] = {0};
static cond_id_t can_read[NUM_TERMINALS];

static cond_id_t newline_entered[NUM_TERMINALS];
static int newlines[NUM_TERMINALS] = {0};



static struct termstat stats[NUM_TERMINALS] = {{0,0,0,0}};

extern void ReceiveInterrupt(int term) {
    Declare_Monitor_Entry_Procedure();

    char c = ReadDataRegister(term);

    printf("Received character %02x\n", c);

    // If either buffer is full, ignore the event.
    if (input_chars[term] == BUF_LEN)
        return;

    printf("Input chars buffer: %d\n", input_chars[term]);

    switch (c){
        case '\b':
        case '\177':
            if (line_chars[term] > 0) {
                input_write_pos[term] = (input_write_pos[term] - 1) % BUF_LEN;
                --input_chars[term];
                --line_chars[term];

                echo(term, "\b \b", 3);

            }
            break;
        case '\r':
        case '\n':
            input_buffer[term][input_write_pos[term]] = '\n';
            input_write_pos[term] = (input_write_pos[term] + 1) % BUF_LEN;
            ++input_chars[term];
            line_chars[term] = 0;
            ++newlines[term];

            echo(term, "\r\n", 2);

            CondSignal(newline_entered[term]);

            break;
        default:
            input_buffer[term][input_write_pos[term]] = c;
            input_write_pos[term] = (input_write_pos[term] + 1) % BUF_LEN;

            if (echo_chars[term] < BUF_LEN) {
                echo_buffer[term][echo_write_pos[term]] = c;
                echo_write_pos[term] = (echo_write_pos[term] + 1) % BUF_LEN;
                ++echo_chars[term];
            }

            ++input_chars[term];
            ++line_chars[term];

            break;

    }

    stats[term].tty_in += 1;

    flush_output(term);

    // Signal input buffer is non-empty.
    CondSignal(inp_empty[term]);
}

extern void TransmitInterrupt(int term) {
    Declare_Monitor_Entry_Procedure();

    printf("Transmit interrupt received\n");

    wait_for_data_reg[term] = 0;
    CondSignal(data_register_ready[term]);
}

extern int WriteTerminal(int term, char *buf, int buflen) {
    Declare_Monitor_Entry_Procedure();

    // Ensure only one thread is writing to the same terminal.
    // This prevents inputs from intermixing.
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

    while (is_reading[term] > 0)
        CondWait(can_read[term]);
    is_reading[term] = 1;

    int i = 0;
    char c = ' ';

    // If there are no unread newlines, wait to read from the input buffer.
    if (newlines[term] == 0)
        CondWait(newline_entered[term]);

    while (i < buflen && c != '\n') {
        if (input_chars[term] == 0)
            CondWait(inp_empty[term]);

        c = (buf[i++] = input_buffer[term][input_read_pos[term]]);
        input_read_pos[term] = (input_read_pos[term] + 1) % BUF_LEN;
        --input_chars[term];
    }

    stats[term].user_out += i;

    is_reading[term] = 0;
    CondSignal(can_read[term]);

    if (c != '\n')
        CondWait(newline_entered[term]);
    else
        --newlines[term];
    CondSignal(newline_entered[term]);

    return i;
}

extern int InitTerminal(int term) {
    Declare_Monitor_Entry_Procedure();

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
        can_read[i] = CondCreate();
        newline_entered[i] = CondCreate();
    }

    return 0;
}

/*
 * Copies the contents of buf into the echo buffer, if possible.
 */
static void echo(int term, char *buf, int buflen) {
    int i;

    if (echo_chars[term] >= BUF_LEN - buflen)
        return;

    for (i = 0; i < buflen; i++)
        echo_buffer[term][echo_write_pos[term] + i] = buf[i];

    echo_chars[term] += i;
    echo_write_pos[term] += i;
}

/*
 * Outputs all the characters in the echo buffer and the output buffer.
 * Characters in the echo buffer will be output first.
 */
static void flush_output(int term) {
    printf("flush_output\n");

    while (output_chars[term] + echo_chars[term] > 0) {

        // If something was written and no transmit interrupt has been received,
        // wait for an interrupt.
        while (wait_for_data_reg[term] > 0)
            CondWait(data_register_ready[term]);

        // Prioritize echoed characters.
        if (echo_chars[term] > 0) {
            printf("Writing character: %02x\n", echo_buffer[term][echo_read_pos[term]]);
            WriteDataRegister(term, echo_buffer[term][echo_read_pos[term]]);
            echo_read_pos[term] = (echo_read_pos[term] + 1) % BUF_LEN;
            echo_chars[term]--;
        } else {
            WriteDataRegister(term, output_buffer[term][output_read_pos[term]]);
            output_read_pos[term] = (output_read_pos[term] + 1) % BUF_LEN;
            output_chars[term]--;
        }

        // Mark output register as occupied.
        wait_for_data_reg[term] = 1;

        stats[term].tty_out += 1;

    }
}