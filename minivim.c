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
文件名称：   minivim.c
项目名称：  mini-vim
创建者：    刘子鹏
创建时间：   2013-10-09
最后修改时间： 2013-10-12
功能：     主程序，实现各种模式
与其他文件的依赖关系：
    头文件 buffer.h regex.h minivim.h
**************************************************/

#include "minivim.h"

// Global varialbles
int cury, curx;             // the current position of the cursor
int topy;                   // the top line number of the current screen
extern int num_lines;       // number of lines: define the available area for the cursor move
extern line_t *head, *cur_line;     // buffer
WINDOW *status_win, *cmd_win;       // windows to show status and command
WINDOW *buffer_pad;                 // buffer window
char cur_file_name[FILENAME_MAX] = "[No Name]";         // current file name
REGEX_RESULT *search_head = NULL, *search_cur = NULL;   // search result



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
            print_file(0);
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
    nonl();
    noecho();
    wresize(stdscr, LINES - 2, COLS);   // Leave two rows for status and command
    keypad(stdscr, TRUE);

    // Create a new pad
    buffer_pad = newpad(BUFFER_LINES, COLS);
    scrollok(buffer_pad, TRUE);
    keypad(buffer_pad, TRUE);
    idlok(buffer_pad, TRUE);
    cury = curx = 0;

    // Create status bar and command bar
    status_win = newwin(1, COLS, LINES - 2, 0);
    wrefresh(status_win);
    update_status();
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
        wmove(buffer_pad, cury - topy, curx);
        prefresh(buffer_pad, 0, 0, 0, 0, BUFFER_LINES - 1, COLS);
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
                if (cury > topy + BUFFER_LINES - 1)
                    update_topy(cury - (topy + BUFFER_LINES - 1));
                break;

            case KEY_UP:
                if (cury > 0) {
                    cury--;
                    cur_line = cur_line->prev;
                    assert(cur_line != NULL);
                    if (curx > cur_line->len)
                        curx = cur_line->len;
                }
                if (cury < topy)
                    update_topy(topy - cury);
                break;

            case KEY_RIGHT:
                if (curx < cur_line->len) curx++;
                break;

            case 10:
            case 13:
            case KEY_ENTER:
                rem_str = &(cur_line->content[curx]);
                old_curx = curx;
                wclrtoeol(buffer_pad);
                wmove(buffer_pad, ++cury, curx = 0);
                winsertln(buffer_pad);
                prefresh(buffer_pad, cury - topy, 0, 0, 0, BUFFER_LINES - 1, COLS);
                if (rem_str != NULL && strlen(rem_str) > 0) {
                    wprintw(buffer_pad, "%s", rem_str);
                    prefresh(buffer_pad, cury - topy, 0, 0, 0, BUFFER_LINES - 1, COLS);
                }

                add_line_next(cur_line);
                cur_line = cur_line->next;
                add_string(cur_line, 0, rem_str);
                del_string_to_eol(cur_line->prev, old_curx);
                break;

            case KEY_DEL:
                if (curx) {
                    wmove(buffer_pad, cury, --curx);
                    wdelch(buffer_pad);
                    del_char(cur_line, curx);
                    /* mvwprintw(buffer_pad, cury - topy, 0, "%s", cur_line->content); */
                    /* prefresh(buffer_pad, cury - topy, 0, 0, 0, BUFFER_LINES - 1, COLS); */

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
                winsch(buffer_pad, c);
                curx++;
                break;
        }
    }
    if (curx) curx--;
    wmove(buffer_pad, cury - topy, curx);
    prefresh(buffer_pad, 0, 0, 0, 0, BUFFER_LINES - 1, COLS);
    werase(cmd_win);
    wrefresh(cmd_win);
}

/* 函数功能描述：进入控制模式，根据输入的命令分发给各个模式，处理光标移动
 */
void control_mode() {
    int c;
    int val = 1, input_number = 0;        // number of lines (characters) for cursor movement

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
        wmove(buffer_pad, cury - topy, curx);
        prefresh(buffer_pad, 0, 0, 0, 0, BUFFER_LINES - 1, COLS);
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
                scroll_lines(val);
                break;

            case 'k':
            case KEY_UP:
                scroll_lines(-val);
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

            case 'd':
            case CTRL_D:
                scroll_lines(BUFFER_LINES / 2);
                break;

            case 'u':
            case CTRL_U:
                scroll_lines(-BUFFER_LINES / 2);
                break;

            case 'f':
            case CTRL_F:
                scroll_lines(BUFFER_LINES - 1);
                break;

            case 'b':
            case CTRL_B:
                scroll_lines(-(BUFFER_LINES - 1));
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
                curx = 0;
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
        assert(search_head != NULL);
        assert(search_head->next != NULL);
        search_cur = search_head->next;
        curx = search_cur->start;
        scroll_lines(search_cur->line - cury);
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
    int p = (cury + 1) * 100 / num_lines;
    mvwprintw(status_win, 0, POS_INFO, "%*d%%", POS_WIDTH - 2, p);
    mvwprintw(status_win, 0, POS_INFO, "%d, %d", cury + 1, curx + 1);   // Count from 1
    wrefresh(status_win);
    wmove(buffer_pad, cury - topy, curx);
    prefresh(buffer_pad, 0, 0, 0, 0, BUFFER_LINES - 1, COLS);
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
    char buf[MAXLEN];
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
void print_file(int start_line) {
    line_t *iter;
    werase(buffer_pad);
    int i = 0;
    for (iter = head->next; iter != NULL && i < start_line; iter = iter->next)
        i++;
    i = 0;
    for (; iter != NULL && i <= BUFFER_LINES - 1; iter = iter->next) {
        if (iter->content != NULL)
            mvwprintw(buffer_pad, i, 0, "%s", iter->content);
        i++;
    }
    prefresh(buffer_pad, 0, 0, 0, 0, BUFFER_LINES - 1, COLS);
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
    if (search_head == NULL || search_head->next == NULL) {
        PRINT_COMMAND_MSG("No match found!");
        return;
    }
    if (search_cur->next == NULL)     // Last match
        search_cur = search_head->next;
    else
        search_cur = search_cur->next;
    assert(search_cur != NULL);
    mvwprintw(cmd_win, 0, 0, "%d %d", search_cur->line, search_cur->start);
    wrefresh(cmd_win);
    wmove(buffer_pad, cury, curx);
    prefresh(buffer_pad, topy, 0, 0, 0, BUFFER_LINES - 1, COLS);
    curx = search_cur->start;
    scroll_lines(search_cur->line - cury);
}

/* 函数功能描述：光标移动到上一个匹配处
 */
void goto_prev_match() {
    if (search_head == NULL || search_head->next == NULL) {
        PRINT_COMMAND_MSG("No match found!");
        return;
    }
    if (search_cur->prev == search_head) {     // First match
        while (search_cur->next != NULL) search_cur = search_cur->next;
    } else
        search_cur = search_cur->prev;
    assert(search_cur != NULL);
    curx = search_cur->start;
    scroll_lines(search_cur->line - cury);
}

/* 函数功能描述：光更新最顶一行的行号以及重新打印屏幕内的内容
 * 参数：delta 更新的幅度值
 */
void update_topy(int delta) {
    topy += delta;
    print_file(topy);
}

/* 函数功能描述：向上、下滚动多行
 * 参数：val 滚动的行数，正数表示向下滚动，负数反之
 */
void scroll_lines(int val) {
    int i;
    if (val > 0) {
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
        if (cury > topy + BUFFER_LINES - 1)
            update_topy(cury - (topy + BUFFER_LINES - 1));
    } else if (val < 0) {
        val = -val;
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
        if (cury < topy)
            update_topy(cury - topy);
    }
}
