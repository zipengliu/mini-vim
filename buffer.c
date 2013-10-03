#include "buffer.h"

void add_char(line_t *line, int pos, int c) {
    assert(line != NULL);
    assert(c < 128);        // Make sure that the character is in [0, 127]

    // Allocate memory
    if (line->content == NULL || line->len >= line->size)
        alloc_chunk(line, 1);

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
    assert(pos > 0);
    assert(pos < line->len);

    if (pos < line->len)
        memmove(line->content + pos, line->content + pos + 1, line->len - pos);
    line->len--;
    line->content[line->len] = 0;

    del_chunk(line);
}

void add_line(line_t *line) {
    assert(line != NULL);
    line_t *new_line = (line_t *) malloc(sizeof(line_t));
    new_line->start = new_line->len = new_line->size = 0;
    new_line->prev = line;
    new_line->next = line->next;
    new_line->content = NULL;
    line->next = new_line;
}

void add_string(line_t *line, int pos, char *str) {

}

void alloc_chunk(line_t *line, int num) {
    assert(line != NULL);
    assert(num >= 0);
    line->size += num * CHUNK_SIZE;
    line->content = realloc(line->content, line->size * sizeof(char));
}

void del_chunk(line_t *line) {
    assert(line != NULL);

    line->size = ROUNDUP(line->len, CHUNK_SIZE);
    if (line->len < line->size - CHUNK_SIZE)
        line->content = realloc(line->content, line->size * sizeof(char));
}