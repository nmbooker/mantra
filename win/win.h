/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * Mantra: Man page bookmarker                                           *
 * Copyright (C) 2015  Tom Pickering                                     *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                       *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef GUARD_WIN_H
#define GUARD_WIN_H

#include <stdbool.h>

#include <ncurses.h>
#include <panel.h>

typedef struct _Win {
    unsigned int r;
    unsigned int c;
    bool can_be_active;
    WINDOW* win;
    PANEL* pnl;
    void (*draw)();
    void (*input)(int);
    void (*update)();
} Win;

extern const int WIN_COL_PAIR_NORMAL;
extern const int WIN_COL_PAIR_ACTIVE;
extern const int WIN_COL_PAIR_PANELS;
extern const int WIN_COL_PAIR_BOOKMARK_HL;
extern const int WIN_COL_PAIR_PAGE_HL;

extern const int WIN_IDX_BOOKMARKS;
extern const int WIN_IDX_PAGES;
extern const int WIN_IDX_HELPBAR;
extern const int WIN_IDX_BOOKPNL;
extern const int WIN_IDX_SEARCHPNL;
extern const int NWIN;

extern Win** wins;
extern int win_act_idx;
extern int pnl_act_idx;

void win_init_all();
void pnl_init_all();
void win_update(Win*, int, int, int, int);
void win_clear_row(Win*, int);
void win_clear_all();
void win_cycle_active();
void win_set_active(int);
int  win_active();
void win_draw_border(Win*);
void win_draw_all();
void draw_win_bookmarks();
void draw_win_pages();
void draw_win_helpbar();
void draw_win_bookpnl();
void draw_win_searchpnl();
void input_win_bookmarks(int);
void input_win_pages(int);
void input_win_bookpnl(int);
void input_win_searchpnl(int);
void search_pagewin(bool, char*);
void search_bmwin(bool, char*);
void update_win_pages();
void update_win_bookmarks();
Win* active_win();
Win* active_pnl();
char* string_clean_buffer(char*, char*, unsigned int);
void open_page(int, char*, char*);
void open_panel(int);
void close_panel(void);

#endif
