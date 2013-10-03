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
        line->content = realloc(line->content, line->size * sizeof(char));
    }
}

void del_chunk(line_t *line) {
    assert(line != NULL);

    line->size = ROUNDUP(line->len, CHUNK_SIZE);
    if (line->len < line->size - CHUNK_SIZE)    // To avoid reallocating memory frequently, leave one chunk for future use
        line->content = realloc(line->content, line->size * sizeof(char));
}
