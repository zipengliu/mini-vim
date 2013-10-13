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

/*************************************************
文件名称：   buffer.c
项目名称：  mini-vim
创建者：    刘子鹏
创建时间：   2013-10-09
最后修改时间： 2013-10-12
功能：     实现缓冲区底层功能
与其他文件的依赖关系：
    头文件 buffer.h
**************************************************/

#include "buffer.h"

int num_lines;
line_t *head, *cur_line;    // head is not used for storing data

void initialize_buffer() {
    num_lines = 1;
    head = (line_t*) malloc(sizeof(line_t));
    head->prev = head->next = NULL;
    head->start = head->len = head->size = 0;
    head->content = NULL;           // Must be initialized!
    cur_line = head;
}

void add_char(line_t *line, int pos, int c) {
    assert(line != NULL);
    assert(c < 128);        // Make sure that the character is in [0, 127]

    // Allocate memory
    alloc_chunk(line, line->len + 1);

    // Make room for the inserted character.  Use memmove rather than
    // memcpy since the latter cannot deal with overlap correctly.
    if (pos < line->len)
        memmove(line->content + pos + 1, line->content + pos, line->len - pos);
    line->content[pos] = c;
    line->len++;
    line->content[line->len] = 0;     // Make sure that it ends with a '\0'
}

void del_char(line_t *line, int pos) {
    assert(line != NULL);
    assert(pos >= 0);
    assert(pos < line->len);

    if (pos < line->len)
        memmove(line->content + pos, line->content + pos + 1, line->len - pos);
    line->len--;
    line->content[line->len] = 0;

    del_chunk(line);
}

void add_line_next(line_t *line) {
    assert(line != NULL);
    line_t *new_line = (line_t *) malloc(sizeof(line_t));
    new_line->start = new_line->len = new_line->size = 0;
    new_line->prev = line;
    new_line->next = line->next;
    new_line->content = NULL;
    if (line->next != NULL)
        (line->next)->prev = new_line;
    line->next = new_line;
    num_lines++;
}

void add_line_prev(line_t *line) {
    assert(line != NULL);
    line_t *new_line = (line_t *) malloc(sizeof(line_t));
    new_line->start = new_line->len = new_line->size = 0;
    new_line->content = NULL;
    new_line->next = line;
    new_line->prev = line->prev;
    (line->prev)->next = new_line;
    line->prev = new_line;
    num_lines++;
}

int del_lines(int start, int end) {
    if (start < 1 || start > end || end > num_lines)    // validate the line range
        return -1;
    line_t *s, *e;
    int i;
    for (i = 0, s = head; i < start; i++)
        s = s->next;
    for (i = 0, e = head; i < end; i++)
        e = e->next;
    if (s == NULL || e == NULL)
        return -1;
    s->prev->next = e->next;
    if (e->next != NULL) {
        e->next->prev = s->prev;
        cur_line = e->next;
    } else
        cur_line = s->prev;

    for (; s != e; s = s->next)         // Free memory
        free(s);
    free(e);

    if (head->next == NULL) {           // All lines are deleted, need to do some amending work: re-adding a node for the first line
        line_t *tmp = (line_t*) malloc(sizeof(line_t));
        tmp->start = tmp->len = tmp->size = 0;
        head->next = tmp;
        tmp->prev = head;
        tmp->content = NULL;
        tmp->next = NULL;
    }

    num_lines -= end - start + 1;
    if (num_lines == 0)
        num_lines = 1;
    return 0;
}

void add_string(line_t *line, int pos, char *str) {
    assert(line != NULL);
    if (str == NULL) return;
    int len = strlen(str);
    if (!len) return;
    alloc_chunk(line, line->len + len);
    if (pos < line->len)
        memmove(line->content + pos, line->content + pos + len, line->len - pos);
    memmove(line->content + pos, str, len);
    line->len += len;
    line->content[line->len] = 0;
}

void del_string_to_eol(line_t *line, int pos) {
    assert(line != NULL);
    assert(pos >= 0);
    if (pos < line->len) {
        line->len = pos;
        line->content[pos] = 0;
        del_chunk(line);
    }
}

void alloc_chunk(line_t *line, int len) {
    assert(line != NULL);
    int new_size = ROUNDUP(len, CHUNK_SIZE);
    if (new_size > line->size) {
        line->size = new_size;
        line->content = (char *) realloc(line->content, line->size * sizeof(char));
    }
}

void del_chunk(line_t *line) {
    assert(line != NULL);

    line->size = ROUNDUP(line->len, CHUNK_SIZE);
    if (line->len < line->size - CHUNK_SIZE)    // To avoid reallocating memory frequently, leave one chunk for future use
        line->content = (char *) realloc(line->content, line->size * sizeof(char));
}
