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

#ifndef __MINIVIM_BUFFER__
#define __MINIVIM_BUFFER__ 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ncurses.h>

#define ROUNDUP(a, n)           \
({                              \
    int x = (a) + (n) - 1;      \
    (x) - (x) % (n);            \
})

#define CHUNK_SIZE  100     // the size of a chunk, used for the buffer: allocate another CHUNK_SIZE bytes if running out of memory

typedef struct line_t {     // buffer structure: double-linked list
    char *content;
    int start, len, size;
    struct line_t *prev, *next;
} line_t;

void initialize_buffer();
void add_char(line_t *line, int pos, int c);
void del_char(line_t *line, int pos);

void add_line_next(line_t *line);
void add_line_prev(line_t *line);
int del_lines(int start, int end);

void add_string(line_t *line, int pos, char *str);
void del_string_to_eol(line_t *line, int pos);

void alloc_chunk(line_t *line, int len);
void del_chunk(line_t *line);



#endif
