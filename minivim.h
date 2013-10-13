    // 我真诚地保证：
// 我自己独立地完成了整个程序从分析、设计到编码的所有工作。
// 如果在上述过程中，我遇到了什么困难而求教于人，那么，我将在程序实习报告中
// 详细地列举我所遇到的问题，以及别人给我的提示。
// 我的程序里中凡是引用到其他程序或文档之处，
// 例如教材、课堂笔记、网上的源代码以及其他参考书上的代码段,
// 我都已经在程序的注释里很清楚地注明了引用的出处。
// 我从未抄袭过别人的程序，也没有盗用别人的程序，
// 不管是修改式的抄袭还是原封不动的抄袭。
    // 我编写这个程序，从来没有想过要去破坏或妨碍其他计算机系统的正常运转。
    // 刘子鹏

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

#define KEY_ESC 27
#define KEY_DEL 127
#define CTRL_D 4
#define CTRL_U 21
#define CTRL_F 6
#define CTRL_B 2

#define POS_WIDTH (COLS / 5)
#define POS_INFO (4 * COLS / 5)
#define BUFFER_LINES (LINES - 2)

#define COMMAND_LENGTH 1000
#define MAXLEN 128

const int MAX_LENGTH_FILENAME = 1000;

// Utility functions
void initialize_screens();
void destroy_screens(int exit_code);
int save_file(const char *file_name);     // Save the buffer to the file
int read_file(const char *file_name);     // Read file into buffer and print the content
void print_file(int start_line);
void update_status();     // Update the status bar (filename and cursor position).  If filename is NULL, only position is updated
int is_number(const char* st);
int input_command(char *cmd);
void goto_next_match();
void goto_prev_match();
void update_topy(int delta);
void scroll_lines(int n);

// Modes functions
void insert_mode();         // Hit i/I/a/A/o/O to enter in control mode
void control_mode();        // Hit escape to enter or return from command mode
void command_mode();        // Hit colon to enter in control mode
void search_mode();

#endif
