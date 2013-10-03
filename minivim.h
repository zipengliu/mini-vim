#ifndef __MINIVIM__

#define __MINIVIM__ 1

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "buffer.h"

// TODO: Althogh the Alt is also 27, ignore it for now
#define KEY_ESC 27
#define KEY_DEL 127

#define POS_WIDTH (COLS / 5)
#define POS_INFO (4 * COLS / 5)

#define COMMAND_LENGTH 1000

const int MAX_LENGTH_FILENAME = 1000;

// Utility functions
void initialize_screens();
void destroy_screens(int exit_code);
int save_file(const char *file_name);     // Save the buffer to the file
int read_file(const char *file_name);     // Read file into buffer and print the content
void update_status(const char *filename);     // Update the status bar (filename and cursor position).  If filename is NULL, only position is updated

// Modes functions
void insert_mode();         // Hit i/I/a/A/o/O to enter in control mode
void control_mode();        // Hit escape to enter or return from command mode
void command_mode();        // Hit colon to enter in control mode

#endif
