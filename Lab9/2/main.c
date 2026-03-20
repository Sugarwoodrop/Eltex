#include "panel.h"

void print_dir_and_file(WINDOW *win, struct Panel panel){
    wattron(win, COLOR_PAIR(3)  | A_BOLD);
    wprintw(win, "%s\n",panel.path);
    wattroff(win, COLOR_PAIR(3) | A_BOLD);
    for(int i = 0; i < panel.count_dir; i++){
        if(panel.selected == i){
            wattron(win, COLOR_PAIR(1) | A_REVERSE| A_BOLD);
            wprintw(win, "/%s\n",panel.dir[i]);
            wattroff(win, COLOR_PAIR(1) | A_REVERSE | A_BOLD);
            continue;
        }
        wprintw(win, "/%s\n",panel.dir[i]);
    }
    for(int i = 0; i < panel.count_file; i++){
         if(panel.selected == i+panel.count_dir){
            wattron(win, COLOR_PAIR(1) | A_REVERSE| A_BOLD);
            wprintw(win, "%s\n",panel.file[i]);
            wattroff(win, COLOR_PAIR(1) | A_REVERSE | A_BOLD);
            continue;
        }
        wprintw(win, "%s\n",panel.file[i]);
    }
}

void update_win(WINDOW **win, struct Panel *panel){
        wclear(win[0]);
        wclear(win[1]);
        wmove(win[0], 0, 0);
        wmove(win[1], 0, 0);

        print_dir_and_file(win[0], panel[0]);
        print_dir_and_file(win[1], panel[1]);

        wrefresh(win[0]);
        wrefresh(win[1]);
}

int main(){
    struct Panel panel[2];
    int error = initPanel(&panel[0]);
    if(error == ERROR){
        return ERROR;
    }
    error = initPanel(&panel[1]);
    if(error == ERROR){
        return ERROR;
    }
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
     if (!has_colors()) {
        endwin();
        return 1;
    }
    start_color();
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    WINDOW *win[2];
    win[0] = newwin(rows, cols/2, 0, 0);
    win[1] = newwin(rows, cols/2, 0, cols/2);
    keypad(win[0], TRUE);
    keypad(win[1], TRUE);
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_YELLOW);
    wbkgd(win[0], COLOR_PAIR(1));
    wbkgd(win[1], COLOR_PAIR(2));  
    int ch;
    int numpanel = 0;
    while(1) {

        update_win(win, panel);

        ch = wgetch(win[numpanel]);
        if(ch == 'q'){
            break;
        }
        if(ch == KEY_UP){
            prev_file(&panel[numpanel]);
        }
        if(ch == KEY_DOWN){
            next_file(&panel[numpanel]);
        }
        if(ch == KEY_ENTER || ch == '\n' || ch == '\r'){
            if(panel[numpanel].selected < panel[numpanel].count_dir){
                error = openDir(&panel[numpanel]);
                if(error == ERROR){ // Первая удалилась после ошибки, надо вторую удолить
                    numpanel++;
                    if(numpanel > 1){
                        numpanel = 0;
                    }
                    delitePanel(&panel[numpanel]);
                    
                    delwin(win[0]);
                    delwin(win[1]);
                    return ERROR;
                }
            }
        }
        if(ch == '\t'){
            wbkgd(win[numpanel], COLOR_PAIR(2));
            numpanel++;
            if(numpanel > 1){
                numpanel = 0;
            }
            chdir(panel[numpanel].path);
            wbkgd(win[numpanel], COLOR_PAIR(1));  
            update_win(win, panel);
        }
    }
    delwin(win[0]);
    delwin(win[1]);
    delitePanel(&panel[0]);
    delitePanel(&panel[1]);
    endwin();
    return 0;
}
