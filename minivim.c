// TODO: how to scroll horizontally and vertically
//       support cut/copy/paste
//       catch Ctrl-C
#include "minivim.h"

// Global varialbles
int cury, curx;             // the current position of the cursor
extern int num_lines;       // number of lines: define the available area for the cursor move
extern line_t *head, *cur_line;
WINDOW *status_win, *cmd_win;


int main(int argc, const char *argv[])
{
    const char USAGE[] = "USAGE: minivim [file]";

    if (argc == 1) {                    // Create a new file
        initialize_screens();
        control_mode();
        destroy_screens(0);
    } else if (argc == 2) {
        if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {   // print help
            printf("%s\n", USAGE);
        } else {                        // Edit the file specified
            initialize_screens();
            read_file(argv[1]);
            control_mode();

            destroy_screens(0);
        }

    } else if (argc > 2) {              // Error number of arguments
        printf("%s\n", USAGE);
    }

    return 0;
}

void initialize_screens() {
    num_lines = 1;
    head = (line_t*) malloc(sizeof(line_t));
    head->prev = head->next = NULL;
    head->start = head->len = head->size = 0;
    head->content = NULL;           // Must be initialized!
    cur_line = head;

    initscr();
    cbreak();
    nonl();     // FIXME: the enter keystroke
    noecho();
    scrollok(stdscr, TRUE);
    idlok(stdscr, TRUE);
    keypad(stdscr, TRUE);
    wresize(stdscr, LINES - 2, COLS);   // Leave two rows for status and command
    getbegyx(stdscr, cury, curx);       // Set the initial cursor position

    // Create status bar and command bar
    status_win = newwin(1, COLS, LINES - 2, 0);
    wrefresh(status_win);
    update_status("[No Name]");
    /* box(status_win, 0, 0); */
    /* wrefresh(status_win); */
    cmd_win = newwin(1, COLS, LINES - 1, 0);
    wrefresh(cmd_win);
}

void destroy_screens(int exit_code) {
    delwin(status_win);
    delwin(cmd_win);
    endwin();
    exit(exit_code);
}

void insert_mode() {
    int c, old_curx;
    char *rem_str = NULL;   // string to the right of the cursor

    werase(cmd_win);
    wprintw(cmd_win, "-- INSERT --");
    wrefresh(cmd_win);
    while (1) {
        update_status(NULL);
        move(cury, curx);
        refresh();
        c = getch();
        if (c == KEY_ESC) break;

        switch (c) {
            case KEY_LEFT:
                if (curx) curx--;
                break;

            case KEY_DOWN:
                if (cury < num_lines - 1) {
                    cury++;
                    cur_line = cur_line->next;
                    assert(cur_line != NULL);
                    if (curx > cur_line->len)
                        curx = cur_line->len;
                }
                break;

            case KEY_UP:
                if (cury) {
                    cury--;
                    cur_line = cur_line->prev;
                    assert(cur_line != NULL);
                }
                break;

            case KEY_RIGHT:
                if (curx < cur_line->len) curx++;
                break;

            case 10:
            case 13:
            case KEY_ENTER:
                rem_str = &(cur_line->content[curx]);
                old_curx = curx;
                clrtoeol();
                move(++cury, curx = 0);
                insertln();
                refresh();
                if (rem_str != NULL && strlen(rem_str) > 0) {
                    printw("%s", rem_str);
                    refresh();
                }

                add_line_next(cur_line);
                cur_line = cur_line->next;
                add_string(cur_line, 0, rem_str);
                del_string_to_eol(cur_line->prev, old_curx);
                break;

            case KEY_DEL:
                if (curx) {
                    curx--;
                    mvdelch(cury, curx);
                    del_char(cur_line, curx);
                } else {
                    if (cury) {     // Move up one line
                        cur_line = cur_line->prev;
                        cury--;
                        curx = cur_line->len;
                    }
                }
                break;

            default:
                add_char(cur_line, curx, c);
                insch(c);
                curx++;
                break;
        }
    }
    werase(cmd_win);
    wrefresh(cmd_win);
}

void control_mode() {
    int c;
    while (1) {
        move(cury, curx);
        refresh();
        update_status(NULL);
        c = getch();
        switch (c) {
            case 'h':
            case KEY_LEFT:
            case KEY_DEL:
                if (curx) curx--;
                break;

            case 'j':
            case KEY_DOWN:
            case KEY_ENTER:
                if (cury < num_lines - 1) {
                    cury++;
                    cur_line = cur_line->next;
                    assert(cur_line != NULL);
                    if (curx > cur_line->len)
                        curx = cur_line->len;
                }
                break;

            case 'k':
            case KEY_UP:
                if (cury) {
                    cury--;
                    cur_line = cur_line->prev;
                    assert(cur_line != NULL);
                }
                break;

            case 'l':
            case KEY_RIGHT:
                if (curx < cur_line->len) curx++;
                break;

            case ':':
                command_mode();
                break;

            case 'i':
                insert_mode();
                break;

            case 'o':
                move(++cury, curx = 0);
                insertln();
                add_line_next(cur_line);
                cur_line = cur_line->next;
                insert_mode();
                break;

            case 'O':
                break;

            default:
                break;
        }
    }
}

void command_mode() {
    int c, ret_val, len = 0;
    char cmd[COMMAND_LENGTH];
    char filename[FILENAME_MAX];
    werase(cmd_win);
    mvwprintw(cmd_win, 0, 0, ":");
    wrefresh(cmd_win);
    while (c = getch(), c != KEY_ENTER && c != 10 && c != 13) {
        if (c == KEY_DEL) {
            if (getcurx(cmd_win) > 1) {
                mvwdelch(cmd_win, 0, getcurx(cmd_win) - 1);
                wrefresh(cmd_win);
                len--;
            }
        } else if (c < 128) {
            waddch(cmd_win, c);
            wrefresh(cmd_win);
            cmd[len++] = c;
        }
    }
    cmd[len] = 0;

    switch(cmd[0]) {
        case 'w':
            strncpy(filename, cmd + 2, FILENAME_MAX);
            ret_val = save_file(filename);
            if (ret_val < 0) {      // Fail to write
                mvwprintw(cmd_win, 0, 0, "Cannot write to \"%s\"", filename);
                wrefresh(cmd_win);
                move(cury, curx);
                refresh();
            }
            break;

        case 'q':
            break;

        default:
            break;

    }

    // Clear the command bar
    werase(cmd_win);
}

void update_status(const char *filename) {
    if (filename != NULL) {
        werase(status_win);
        mvwprintw(status_win, 0, 0, "%s", filename);
    }
    if (cur_line->content != NULL) {     // For debug
        werase(cmd_win);
        mvwprintw(cmd_win, 0, (COLS - cur_line->len) / 2, "%s", cur_line->content);
        wrefresh(cmd_win);
    }
    mvwprintw(status_win, 0, POS_INFO, "%*d%%", POS_WIDTH - 2, (cury + 1) * 100 / num_lines);
    mvwprintw(status_win, 0, POS_INFO, "%d, %d", cury+1, curx+1);   // Count from 1 for readability
    wrefresh(status_win);
    move(cury, curx);
    refresh();
}

int save_file(const char *file_name) {
    FILE *f = fopen(file_name, "w");
    if (f == NULL)
        return -1;
    line_t *iter;
    for (iter = head; iter != NULL; iter = iter->next) {
        if (iter->content != NULL)
            fprintf(f, "%s\n", iter->content);
        else
            fprintf(f, "\n");
    }
    fclose(f);

    update_status(file_name);
    return 0;
}

int read_file(const char *file_name) {
    return 0;
}
