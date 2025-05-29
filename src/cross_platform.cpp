#include <cross_platform.hpp>

#if defined(_WIN64)
#include <windows.h>

#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

std::pair<int, int> get_rowColCount() {
#if defined(_WIN64)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

    return std::make_pair(static_cast<int>(csbi.dwSize.Y), static_cast<int>(csbi.dwSize.X));

#elif defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
    winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        if (ws.ws_row == 0 || ws.ws_col == 0) { return std::make_pair(0, 0); }
        else { return std::make_pair(static_cast<int>(ws.ws_row), static_cast<int>(ws.ws_col)); }
    }
    { return std::make_pair(0, 0); }
#endif
}

bool is_inTerminal() {
    auto sizePair = get_rowColCount();
    if (sizePair.first < 1 || sizePair.second < 1) { return false; }
    else { return true; }
}
