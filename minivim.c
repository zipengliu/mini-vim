// TODO: how to scroll horizontally and vertically
//       how to catch Ctrl-d, etc
#include "minivim.h"

// Global varialbles
int cury, curx;             // the current position of the cursor
extern int num_lines;       // number of lines: define the available area for the cursor move
extern line_t *head, *cur_line;     // buffer
WINDOW *status_win, *cmd_win;       // windows to show status and command
char cur_file_name[FILENAME_MAX] = "[No Name]";         // current file name
REGEX_RESULT *search_head = null, *search_cur = null;   // search result


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

/* 函数功能描述：初始化屏幕
 */
void initialize_screens() {
    initscr();
    cbreak();
    nonl();     // FIXME: the enter keystroke
    noecho();
    scrollok(stdscr, TRUE);
    setscrreg(0, LINES - 2);
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

/* 函数功能描述：销毁屏幕
 * 参数：exit_code 退出代码号
 */
void destroy_screens(int exit_code) {
    delwin(status_win);
    delwin(cmd_win);
    endwin();
    exit(exit_code);
}

/* 函数功能描述：进入插入模式，可处理字符输入、删除，光标上下移动，换行
 */
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

/* 函数功能描述：进入控制模式，根据输入的命令分发给各个模式，处理光标移动
 */
void control_mode() {
    int c;
    int val = 1, input_number = 0;        // number of lines (characters) for cursor movement
    int i;

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
                if (curx - val + 1> 0)
                    curx -= val;
                else
                    curx = 0;
                break;

            case 'j':
            case KEY_DOWN:
            case KEY_ENTER:
                if (cury < num_lines - val) {
                    cury += val;
                    for (i = 0; i < val; i++)
                        cur_line = cur_line->next;
                    assert(cur_line != NULL);
                } else {
                    cury = num_lines - 1;
                    while (cur_line->next != NULL)
                        cur_line = cur_line->next;
                }
                if (cur_line->len == 0)
                    curx = 0;
                else if (curx > cur_line->len - 1)
                    curx = cur_line->len - 1;
                break;

            case 'k':
            case KEY_UP:
                if (cury - val + 1 > 0) {
                    cury -= val;
                    for (i = 0; i < val; i++)
                        cur_line = cur_line->prev;
                    assert(cur_line != NULL);
                } else {
                    cury = 0;
                    cur_line = head->next;
                }
                if (cur_line->len == 0)
                    curx = 0;
                else if (curx > cur_line->len - 1)
                    curx = cur_line->len - 1;
                break;

            case 'l':
            case KEY_RIGHT:
                if (curx < cur_line->len - val)
                    curx += val;
                else {
                    if (cur_line->len == 0)
                        curx = 0;
                    else
                        curx = cur_line->len - 1;
                }
                break;

            case ':':
                command_mode();
                break;

            case '/':
                search_mode();
                break;

            case 'n':
                goto_next_match();
                break;

            case 'N':
                goto_prev_match();
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

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (!input_number) val = 0;
                val = val * 10 + c - 48;
                input_number = 1;
                break;

            default:
                // TODO: warn the user for unkown command
                break;
        }
        if (c < '0' || c > '9') {
            val = 1;
            input_number = 0;
        }
    }
}

/* 函数功能描述：进入命令模式或搜索模式后，通过该函数获取命令或模式串
 * 参数：cmd 用于存储输入的字符串
 * 返回值：-1 输入失败，字符串超过允许长度；>0 字符串长度
 */
int input_command(char *cmd) {
    int c, len = 0;
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
        if (len > COMMAND_LENGTH) {
            PRINT_COMMAND_MSG("Command too long!");
            return -1;
        }
    }
    cmd[len] = 0;
    return len;
}

/* 函数功能描述：进入命令模式，处理输入的各个合法命令
 */
void command_mode() {
    int ret_val, len = 0;
    int taken = 0;          // indicator of whether the command is executed
    int i;
    char cmd[COMMAND_LENGTH];
    char *action = NULL, *filename;
    werase(cmd_win);
    mvwprintw(cmd_win, 0, 0, ":");
    wrefresh(cmd_win);
    len = input_command(cmd);
    if (len < 0) return;

    action = strtok(cmd, " ,");
    assert(action != NULL);
    if (action[0] == 'w') {                 // Save file
        taken = 1;
        filename = strtok(NULL, " ");
        if (filename == NULL) {
            if (!strcmp(cur_file_name, "[No Name]")) {
                PRINT_COMMAND_MSG("No file name specified!");
                return;
            }
            filename = cur_file_name;
        }
        ret_val = save_file(filename);
        if (ret_val < 0) {
            PRINT_COMMAND_MSG("Cannot write to this file!");
            return;
        }
        strncpy(cur_file_name, filename, FILENAME_MAX);
    }

    if (action[0] == 'q' || action[1] == 'q') {     // Exit minivim
        taken = 1;
        if (strchr(action, '!') != NULL || action[0] == 'w') {      // Force exit
            destroy_screens(0);
            exit(0);
        } else {
            // TODO: how to decide whether the file is modified?
            destroy_screens(0);
            exit(0);
        }
    }

    if (is_number(action)) {
        int start_line = atoi(action);
        action = strtok(NULL, " ,");
        if (is_number(action)) {
            int end_line = atoi(action);
            action = strtok(NULL, " ,");
            if (action[0] == 'd') {
                taken = 1;
                ret_val = del_lines(start_line, end_line);  // Clear the buffer of the lines affected
                if (ret_val < 0) {
                    PRINT_COMMAND_MSG("Invalid line range!");
                    return;
                } else {                                      // Clear the lines on screen
                    len = end_line - start_line + 1;
                    move(start_line - 1, 0);
                    for (i = 0; i < len; i++)
                        deleteln();
                    cury = end_line - len;
                    curx = 0;
                    move(cury, curx);
                    refresh();
                }
            }
        }
    }

    if (!taken) {
        PRINT_COMMAND_MSG("Command not found!");
        return;
    }
}

/* 函数功能描述：进入搜索模式，调用正则表达式匹配并定位到第一个匹配
 */
void search_mode() {
    int ret_val, len = 0;
    char regex[COMMAND_LENGTH];
    werase(cmd_win);
    mvwprintw(cmd_win, 0, 0, "/");
    wrefresh(cmd_win);
    len = input_command(regex);

    ret_val = regex_match(head, regex, len, search_head);
    if (ret_val == -1) {
        PRINT_COMMAND_MSG("Invalid regular expression!");
        return;
    } else if (ret_val == 0){
        PRINT_COMMAND_MSG("No match found!");
        return;
    } else {
        assert(search_head != null);
        assert(search_head->next != null);
        search_cur = search_head->next;
        cury = search_cur->line;
        curx = search_cur->start;
    }
}

/* 函数功能描述：更新状态窗口的信息（行列信息，文件名，调试信息等）
 */
void update_status() {
    assert(cur_line != NULL);
    werase(status_win);
    mvwprintw(status_win, 0, 0, "%s", cur_file_name);
    if (cur_line->content != NULL) {     // For debug
        wmove(cmd_win, 0, COLS / 2);
        wclrtoeol(cmd_win);
        mvwprintw(cmd_win, 0, COLS / 2, "%s", cur_line->content);
        wrefresh(cmd_win);
    }
    mvwprintw(status_win, 0, POS_INFO, "%*d%%", POS_WIDTH - 2, (cury + 1) * 100 / num_lines);
    mvwprintw(status_win, 0, POS_INFO, "%d, %d", cury+1, curx+1);   // Count from 1 for readability
    wrefresh(status_win);
    move(cury, curx);
    refresh();
}

/* 函数功能描述：保存文件
 * 参数：file_name 文件名
 * 返回值：-1 写文件失败；0 写文件成功
 */
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

/* 函数功能描述：读取文件并保存到buffer中
 * 参数：file_name 文件名
 * 返回值：-1 读文件失败；0 读文件成功
 */
int read_file(const char *file_name) {
    FILE *f = fopen(file_name, "r");
    if (f == NULL)
        return -1;

    strncpy(cur_file_name, file_name, FILENAME_MAX);
    num_lines = 0;
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

/* 函数功能描述：把读取到的文件打印到屏幕
 */
void print_file() {
    line_t *iter;
    int ret;
    werase(stdscr);
    cury = 0;
    move(cury, curx);
    refresh();
    for (iter = head->next; iter != NULL; iter = iter->next) {
        if (iter->content != NULL)
            printw("%s", iter->content);
        /* cury++; */
        wscrl(stdscr, 1);
        ret = move(cury, curx);
        assert(ret == OK);
        refresh();
    }
    /* move(cury = 0, curx = 0); */
    ret = wscrl(stdscr, -num_lines);
    assert(ret == OK);
    refresh();

    ret = wscrl(stdscr, 20);
    assert(ret == OK);
    refresh();

    curx = getcurx(stdscr);
    cury = getcury(stdscr);
    update_status();
}

/* 函数功能描述：判断一个字符串是否是数字
 * 参数：st 需要判断的字符串
 * 返回：0 不是数字；1 是数字
 */
int is_number(const char *st) {
    if (st == NULL || strlen(st) == 0)
        return 0;
    int i;
    for (i = 0; i < strlen(st); i++) {
        if (st[i] < '0' || st[i] > '9')
            return 0;
    }
    return 1;
}

/* 函数功能描述：光标移动到下一个匹配处
 */
void goto_next_match() {
    if (search_head == null || search_head->next == null) {
        PRINT_COMMAND_MSG("No match found!");
        return;
    }
    if (search_cur->next == null)     // Last match
        search_cur = search_head->next;
    else
        search_cur = search_cur->next;
    cury = search_cur->line;
    curx = search_cur->start;
}

/* 函数功能描述：光标移动到上一个匹配处
 */
void goto_prev_match() {
    if (search_head == null || search_head->next == null) {
        PRINT_COMMAND_MSG("No match found!");
        return;
    }
    if (search_cur->prev == search_head) {     // First match
        while (search_cur->next != null) search_cur = search_cur->next;
    } else
        search_cur = search_cur->prev;
    cury = search_cur->line;
    curx = search_cur->start;
}
