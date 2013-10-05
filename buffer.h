// Buffer manipulations

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
