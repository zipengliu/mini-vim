// TODO: add a limit line
//       how to scroll horizontally
//       add a status window on the bottom to show the filename and status info
//       support cut/copy/paste
#include "minivim.h"

// Global varialbles
int cury, curx;             // the current position of the cursor
int num_lines;              // number of lines: define the available area for the cursor move
line_t *head, *cur_line;    // buffer
WINDOW *status_win, *command_win;


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
    initscr();
    cbreak();
    nonl();     // FIXME: the enter keystroke
    noecho();
    scrollok(stdscr, TRUE);
    idlok(stdscr, TRUE);
    keypad(stdscr, TRUE);
    wresize(stdscr, LINES - 2, COLS);   // Leave two rows for status and command
    getbegyx(stdscr, cury, curx);       // Set the initial cursor position

    status_win = newwin(1, COLS, LINES - 2, 0);
    wrefresh(status_win);
    /* box(status_win, 0, 0); */
    /* wrefresh(status_win); */
    command_win = newwin(1, COLS, LINES - 1, 0);
    wrefresh(command_win);

    num_lines = 1;
    head = (line_t*) malloc(sizeof(line_t));
    head->prev = head->next = NULL;
    head->start = head->len = head->size = 0;
    head->content = NULL;           // Must be initialized!
    cur_line = head;
}

void destroy_screens(int exit_code) {
    delwin(status_win);
    delwin(command_win);
    endwin();
    exit(exit_code);
}

void read_file(const char file_name[MAX_LENGTH_FILENAME]) {

}

void insert_mode() {
    int c;

    while (1) {
        move(cury, curx);
        refresh();
        c = getch();
        if (c == KEY_ESC) break;

        switch (c) {
            case KEY_LEFT:
                if (curx) curx--;
                break;

            case KEY_DOWN:
                if (cury < LINES - 1) cury++;
                break;

            case KEY_UP:
                if (cury) cury--;
                break;

            case KEY_RIGHT:
                if (curx < COLS - 1) curx++;
                break;

            case 10:
            case 13:
            case KEY_ENTER:   // TODO: get the remaining string to the next line
                cury++;
                curx = 0;
                break;

            case KEY_DEL:
                if (curx) {
                    curx--;
                    mvdelch(cury, curx);
                    del_char(cur_line, curx);
                } else {    // TODO: go to the end of the previous line

                }
                break;

            default:
                add_char(cur_line, curx, c);
                insch(c);
                curx++;
                break;
        }
        mvwprintw(status_win, 0, COLS / 2, "Current input: %d", c);
        wrefresh(status_win);
    }
}

void control_mode() {
    int c;
    while (1) {
        move(cury, curx);
        refresh();
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
                if (cury < LINES - 1) cury++;
                break;

            case 'k':
            case KEY_UP:
                if (cury) cury--;
                break;

            case 'l':
            case KEY_RIGHT:
                if (curx < COLS - 1) curx++;
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
                add_line(cur_line);
                insert_mode();

            default:
                break;
        }
    }
}

void command_mode() {

}
