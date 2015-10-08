#include "input.h"

#include <stdbool.h>

#include "win/win.h"

bool handle_input(int ch) {
    bool quit = false;
    switch (ch) {
        case K_CYCLE:
            win_cycle_active();
            break;
        case K_QUIT:
            quit = true;
            break;
    }
    return !quit;
}