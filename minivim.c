// TODO: how to scroll horizontally and vertically
//       support cut/copy/paste
//       catch Ctrl-C
#include "minivim.h"

// Global varialbles
int cury, curx;             // the current position of the cursor
extern int num_lines;       // number of lines: define the available area for the cursor move
extern line_t *head, *cur_line;
WINDOW *status_win, *cmd_win;
char cur_file_name[FILENAME_MAX] = "[No Name]";


int main(int argc, const char *argv[])
{
    const char USAGE[] = "USAGE: minivim [file]";

    if (argc == 1) {                    // Create a new file
        initialize_buffer();
        initialize_screens();
        control_mode();
        destroy_screens(0);
    } else if (argc == 2) {
        if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {   // print help
            printf("%s\n", USAGE);
        } else {                        // Edit the file specified
            initialize_buffer();
            if (read_file(argv[1]) < 0) {
                printf("Error opening %s\n", argv[1]);
                exit(1);
            }
            initialize_screens();
            print_file();
            control_mode();

            destroy_screens(0);
        }

    } else if (argc > 2) {              // Error number of arguments
        printf("%s\n", USAGE);
    }

    return 0;
}

void initialize_screens() {
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
    update_status();
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
        update_status();
        move(cury, curx);
        refresh();
        c = getch();
        if (c == KEY_ESC) break;

        switch (c) {
            case KEY_LEFT:
                if (curx > 0) curx--;
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
                if (cury > 0) {
                    cury--;
                    cur_line = cur_line->prev;
                    assert(cur_line != NULL);
                    if (curx > cur_line->len)
                        curx = cur_line->len;
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
    if (curx) curx--;
    move(cury, curx);
    refresh();
    werase(cmd_win);
    wrefresh(cmd_win);
}

void control_mode() {
    int c;

    if (head->next == NULL) {
        line_t *tmp = (line_t*) malloc(sizeof(line_t));
        tmp->start = tmp->len = tmp->size = 0;
        head->next = tmp;
        tmp->prev = head;
        tmp->content = NULL;
        tmp->next = NULL;
    }
    cur_line = head->next;

    while (1) {
        move(cury, curx);
        refresh();
        update_status();
        c = getch();
        switch (c) {
            case 'h':
            case KEY_LEFT:
            case KEY_DEL:
                if (curx > 0) curx--;
                break;

            case 'j':
            case KEY_DOWN:
            case KEY_ENTER:
                if (cury < num_lines - 1) {
                    cury++;
                    cur_line = cur_line->next;
                    assert(cur_line != NULL);
                    if (curx > cur_line->len - 1)
                        curx = cur_line->len - 1;
                }
                break;

            case 'k':
            case KEY_UP:
                if (cury > 0) {
                    cury--;
                    cur_line = cur_line->prev;
                    assert(cur_line != NULL);
                    if (curx > cur_line->len - 1)
                        curx = cur_line->len - 1;
                }
                break;

            case 'l':
            case KEY_RIGHT:
                if (curx < cur_line->len - 1) curx++;
                break;

            case ':':
                command_mode();
                break;

            case 'i':
                insert_mode();
                break;

            case 'I':
                curx = 0;   // TODO: determine the non-space start
                insert_mode();
                break;

            case 'a':
                if (cur_line->len > 0) 
                    curx++;
                insert_mode();
                break;

            case 'A':
                curx = cur_line->len;
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
                insertln();
                move(cury, curx = 0);
                refresh();
                update_status();
                add_line_prev(cur_line);
                cur_line = cur_line->prev;
                insert_mode();
                break;

            default:
                // TODO: warn the user for unkown command
                break;
        }
    }
}

void command_mode() {
    int c, ret_val, len = 0;
    char cmd[COMMAND_LENGTH];
    char *action = NULL, *filename;
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
        if (len > COMMAND_LENGTH)       // command too long!
            PRINT_COMMAND_MSG("Command too long!");
    }
    cmd[len] = 0;

    action = strtok(cmd, " ");
    assert(action != NULL);
    if (action[0] == 'w') {
        filename = strtok(NULL, " ");
        if (filename == NULL) {
            if (!strcmp(cur_file_name, "[No Name]"))
                PRINT_COMMAND_MSG("No file name specified!");
            filename = cur_file_name;
        }
        ret_val = save_file(filename);
        if (ret_val < 0)
            PRINT_COMMAND_MSG("Cannot write to this file!");
        strncpy(cur_file_name, filename, FILENAME_MAX);
    }

    if (action[0] == 'q' || action[1] == 'q') {
        if (strchr(action, '!') != NULL) {      // Force exit
            destroy_screens(0);
            exit(0);
        } else {
            // TODO: how to decide whether the file is modified?
        }

    }
}

void update_status() {
    assert(cur_line != NULL);
    werase(status_win);
    mvwprintw(status_win, 0, 0, "%s", cur_file_name);
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
    for (iter = head->next; iter != NULL; iter = iter->next) {
        if (iter->content != NULL)
            fprintf(f, "%s\n", iter->content);
        else
            fprintf(f, "\n");
    }
    fclose(f);

    strncpy(cur_file_name, file_name, FILENAME_MAX);
    update_status();
    return 0;
}

int read_file(const char *file_name) {
    FILE *f = fopen(file_name, "r");
    if (f == NULL)
        return -1;

    strncpy(cur_file_name, file_name, FILENAME_MAX);
    cur_line = head;
    char buf[MAXLEN];       // FIXME: use a dynamic way to adapt to variable-length row
    while (fgets(buf, MAXLEN, f)) {
        int i = strlen(buf);
        while (--i >= 0) {               // Get rid of the \n
            if (buf[i] == '\n') {
                buf[i] = 0;
                break;
            }
        }
        add_line_next(cur_line);
        cur_line = cur_line->next;
        add_string(cur_line, 0, buf);
    }
    return 0;
}

void print_file() {
    line_t *iter;
    werase(stdscr);
    cury = 0;
    move(cury, curx);
    refresh();
    for (iter = head->next; iter != NULL; iter = iter->next) {
        if (iter->content != NULL)
            printw("%s", iter->content);
        cury++;
        move(cury, curx);
        refresh();
    }
    move(cury = 0, curx = 0);
    refresh();
}
