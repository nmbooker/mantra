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

#define _GNU_SOURCE /* Needed for strcasestr() definition */

#include "bookmarks.h"
#include "win.h"

#include <stdlib.h>
#include <string.h>

#include <ncurses.h>

#include "../input.h"
#include "../page.h"
#include "../file.h"

int _current_row_bm = 0;
int _prev_row_bm = 0;
Bookmark* _bm_start = NULL;
Bookmark* _current_bm = NULL;
int _MAX_NAME_LEN_BM = 20;
char* _bm_search = NULL;

Bookmark* get_current_bm() {
    return _current_bm;
}

void _update_current_bm() {
    int i;
    _current_bm = _bm_start;
    for (i = 0; _current_bm != NULL && i < _current_row_bm; ++i) {
        _current_bm = _current_bm->next;
        if (_current_bm == NULL) break;
    }
}

void win_bookmarks_show(Win* win) {
    /* TODO: Deduplicate with pages code */
    int r = 1;
    int c = 2;
    int col_pair;
    Bookmark* bm = _bm_start;
    int max_desc_len;
    char* name;
    char* desc;
    char sect[2];
    Page* page;

    sect[1] = '\0';

    _MAX_NAME_LEN_BM = win->c / 3;
    max_desc_len = win->c - _MAX_NAME_LEN_BM - 7;
    name = malloc((_MAX_NAME_LEN_BM + 1) * sizeof(char));
    desc = malloc((max_desc_len + 1) * sizeof(char));

    if (_bm_start == NULL) _bm_start = bookmarks;

    for (; bm != NULL && r < win->r - 1; ++r) {
        page = bm->page;
        sect[0] = '0' + page->sect;
        mvwprintw(win->win, r, c, sect);
        string_clean_buffer(name, page->name, _MAX_NAME_LEN_BM);
        string_clean_buffer(desc, page->desc,  max_desc_len);
        mvwprintw(win->win, r, c + 2, name);
        mvwprintw(win->win, r, c + 3 + _MAX_NAME_LEN_BM, desc);
        bm = bm->next;
    }

    /* If we ran out of bookmarks to draw before we
     * got to the end of the window, finish up by
     * wiping any debris from the remaining space. */
    for (; r < win->r - 1; ++r)
        win_clear_row(win, r);

    col_pair = WIN_COL_PAIR_NORMAL;
    if (win == wins[win_act_idx])
        col_pair = WIN_COL_PAIR_BOOKMARK_HL;
    mvwchgat(win->win, _current_row_bm + 1, 1, win->c - 2, A_REVERSE, col_pair, NULL);
    if (_current_row_bm != _prev_row_bm) {
        mvwchgat(win->win, _prev_row_bm + 1, 1, win->c - 2, A_NORMAL, WIN_COL_PAIR_NORMAL, NULL);
        _prev_row_bm = _current_row_bm;
    }

    /* Recalculate _current_bm */
    _update_current_bm();

    free(name);
    free(desc);
}

void draw_win_bookmarks() {
    Win* win = wins[WIN_IDX_BOOKMARKS];
    win_bookmarks_show(win);
    win_draw_border(win);
    wrefresh(win->win);
}

void _page_bm(bool down) {
    Win* win = wins[WIN_IDX_BOOKMARKS];
    Bookmark* bm = _bm_start;
    int i;

    if (!_current_bm) return;

    if (down) {
        /* Search ahead; if the list comes to an end soon,
         * don't jump forward a whole page. */
        for (i = 0; i < ((win->r - 2) << 1) - 1; ++i) {
            bm = bm->next;
            if (bm == NULL) break;
        }

        i -= (win->r - 2);
        for (; _bm_start->next != NULL && i >= 0; --i)
            _bm_start = _bm_start->next;
    } else {
        for (i = 0; _bm_start->prev != NULL && i < win->r - 2; ++i)
            _bm_start = _bm_start->prev;
    }

    _update_current_bm();
}

char _navigate_bm(bool down) {
    Win* win = wins[WIN_IDX_BOOKMARKS];
    if (down) {
        if (_current_bm && _current_bm->next && _current_row_bm + 3 < win->r) {
            ++_current_row_bm;
        } else if (_bm_start && _bm_start->next && _current_bm->next) {
            _bm_start = _bm_start->next;
        } else return 1;
    } else {
        if (_current_row_bm == 0) {
            if (_bm_start && _bm_start->prev) {
                _bm_start = _bm_start->prev;
            } else return 1;
        } else {
            --_current_row_bm;
        }
    }

    _update_current_bm();
    return 0;
}

void _jump_to_end_bm(bool end) {
    Win* win = wins[WIN_IDX_BOOKMARKS];

    if (!_current_bm) return;

    if (end) {
        int i;
        _current_row_bm = win->r - 3;
        if (_bm_start == NULL) return;

        /* Fast-forward to end... */
        while (_bm_start->next != NULL)
            _bm_start = _bm_start->next;

        /* ...And rewind! */
        for (i = 0; _bm_start->prev != NULL && i < win->r - 3; ++i)
            _bm_start = _bm_start->prev;
        for (; i < win->r - 3; ++i)
            --_current_row_bm;

    } else {
        _current_row_bm = 0;
        _bm_start = bookmarks;
    }

    _update_current_bm();
}

void _open_bm() {
    if (_current_bm != NULL) {
        open_page(_current_bm->page->sect,
                  _current_bm->page->name,
                  _current_bm->line);
    }
}

void _delete_bm() {
    Bookmark* prev;
    Bookmark* next;
    Bookmark* to_erase;
    if (_current_bm != NULL) {
        prev = _current_bm->prev;
        next = _current_bm->next;
        to_erase = _current_bm;

        if (next == NULL && prev != NULL) {
            _current_bm = prev;
            _navigate_bm(false);
        } else if (next != NULL) {
            _current_bm = next;
        } else { /* next == NULL && prev == NULL */
            _current_bm = NULL;
        }

        if (_bm_start == to_erase)
            _bm_start = _current_bm;

        erase_bookmark(to_erase);
        _update_current_bm();
    }
}

void search_bmwin(bool down, char* term) {
    Bookmark* start = _current_bm;

    if (term) {
        if (_bm_search) free(_bm_search);
        _bm_search = (char*) malloc(strlen(term) + 1);
        strcpy(_bm_search, term);
    }

    if (_current_bm && _bm_search) {
        if (_navigate_bm(down)) _jump_to_end_bm(!down);
        while (_current_bm && strcasestr(_current_bm->page->name, _bm_search) == NULL) {
            if (_navigate_bm(down)) _jump_to_end_bm(!down);
            if (_current_bm == start) break;
        }
    }
}

void input_win_bookmarks(int ch) {
    bool down;
    switch (ch) {
        case K_FWD:
        case K_BACK:
            down = (ch == K_BACK) ? false : true;
            _page_bm(down);
            break;
        case K_UP:
        case K_DOWN:
            down = (ch == K_UP) ? false : true;
            _navigate_bm(down);
            break;
        case K_HOME:
        case K_END:
            down = (ch == K_HOME) ? false : true;
            _jump_to_end_bm(down);
            break;
        case K_NEXT:
        case K_PREV:
            down = (ch == K_PREV) ? false : true;
            search_bmwin(down, NULL);
            break;
        case K_OPEN:
            _open_bm();
            break;
        case K_BM_DEL:
            _delete_bm();
            break;
    }
}

void update_win_bookmarks() {
    Win* win = wins[WIN_IDX_BOOKMARKS];
    int r = win->r;
    int i;

    if (_current_row_bm > r - 3) {
        for (i = 0; _bm_start->next != NULL && i < _current_row_bm - r + 3; ++i)
            _bm_start = _bm_start->next;
        _current_row_bm = r - 3;
    }
}
