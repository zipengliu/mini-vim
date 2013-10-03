#ifndef __MINIVIM__
#define __MINIVIM__ 1

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include "buffer.h"

// TODO: Althogh the Alt is also 27, ignore it for now
#define KEY_ESC 27
#define KEY_DEL 127

const int MAX_LENGTH_FILENAME = 1000;

// Utility functions
void initialize_screens();
void destroy_screens(int exit_code);
void save_file(const char file_name[MAX_LENGTH_FILENAME]);     // Save the buffer to the file
void read_file(const char file_name[MAX_LENGTH_FILENAME]);     // Read file into buffer and print the content

// Modes functions
void insert_mode();         // Hit i/I/a/A/o/O to enter in control mode
void control_mode();        // Hit escape to enter or return from command mode
void command_mode();        // Hit colon to enter in control mode

#endif
