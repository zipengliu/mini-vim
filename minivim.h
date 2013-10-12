#ifndef __MINIVIM__

#define __MINIVIM__ 1

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "buffer.h"
#include "regex.h"

#define PRINT_COMMAND_MSG(msg)  \
    do {                        \
        werase(cmd_win);        \
        mvwprintw(cmd_win, 0, 0, "%s", msg);    \
        wrefresh(cmd_win);      \
        move(cury, curx);       \
        refresh();              \
    } while (false)

// TODO: Althogh the Alt is also 27, ignore it for now
#define KEY_ESC 27
#define KEY_DEL 127

#define POS_WIDTH (COLS / 5)
#define POS_INFO (4 * COLS / 5)

#define COMMAND_LENGTH 1000
#define MAXLEN 128

const int MAX_LENGTH_FILENAME = 1000;

// Utility functions
void initialize_screens();
void destroy_screens(int exit_code);
int save_file(const char *file_name);     // Save the buffer to the file
int read_file(const char *file_name);     // Read file into buffer and print the content
void print_file();
void update_status();     // Update the status bar (filename and cursor position).  If filename is NULL, only position is updated
int is_number(const char* st);
int input_command(char *cmd);
void goto_next_match();
void goto_prev_match();

// Modes functions
void insert_mode();         // Hit i/I/a/A/o/O to enter in control mode
void control_mode();        // Hit escape to enter or return from command mode
void command_mode();        // Hit colon to enter in control mode
void search_mode();

#endif
