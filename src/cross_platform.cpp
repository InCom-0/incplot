#include <cross_platform.hpp>

#if defined(_WIN64)
#include <windows.h>

#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

bool is_inTerminal() {
#if defined(_WIN64)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    // auto width  = (int)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    // auto height = (int)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);

    if (csbi.dwSize.X == 0 || csbi.dwSize.Y == 0) { return false; }
    else { return true; }

#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
    winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        if (ws.ws_row == 0 || ws.ws_col == 0) { return false; }
        else { return true; }
    }
    else { return false; }
#endif
}