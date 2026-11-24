#include <conio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

enum { WIDTH = 60, HEIGHT = 22 };
typedef struct { int x, y; } Point;
enum Control { MENU, DIRECTION, Y_OFFSET, COLOR, SPEED, SYMBOL };
enum CubeColor { DEFAULT_COLOR, RED, CYAN, YELLOW };
static const struct { const char *name, *escape; } palette[] = {
    [DEFAULT_COLOR] = {"Default", "\033[39m"},
    [RED] = {"Red", "\033[91m"},
    [CYAN] = {"Cyan", "\033[96m"},
    [YELLOW] = {"Yellow", "\033[93m"}
};

static Point project(double x, double y, double z, double angle, double tilt,
                     double centerX) {
    // Turn about Y, then tilt about X so the top comes into view.
    double side = x * cos(angle) + z * sin(angle);
    double depth = z * cos(angle) - x * sin(angle);
    double up = y * cos(tilt) - depth * sin(tilt);
    double distance = 5.0 + y * sin(tilt) + depth * cos(tilt);
    // Terminal cells are roughly twice as tall as they are wide.
    Point point = { (int)lround(centerX + 52 * side / distance),
                    (int)lround(HEIGHT / 2 - 26 * up / distance) };
    return point;
}

static void draw_edge(char frame[HEIGHT][WIDTH], Point a, Point b, char symbol) {
    int dx = b.x - a.x, dy = b.y - a.y;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    for (int i = 0; i <= steps; ++i) {
        double t = steps ? (double)i / steps : 0.0;
        int x = (int)lround(a.x + dx * t);
        int y = (int)lround(a.y + dy * t);
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
            frame[y][x] = symbol;
    }
}

int main(void) {
    HANDLE terminal = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (!GetConsoleMode(terminal, &mode) ||
        !SetConsoleMode(terminal, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
        fputs("Run cube.exe in Windows Terminal.\n", stderr);
        return 1;
    }
    const int edges[12][2] = {
        {0,1}, {1,3}, {3,2}, {2,0}, {4,5}, {5,7}, {7,6}, {6,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };
    ULONGLONG lastTick = GetTickCount64();
    double travelPhase = 0.0;
    const double halfTripSeconds = 3.0;
    const double pi = acos(-1.0);
    double angle = 0.45;
    const double speedStep = 0.25;     // radians per second
    const int minLevel = 1, maxLevel = 8;
    int speedLevel = 4;
    int direction = 1;
    const double acceleration = 2.0;   // radians per second squared
    double speed = speedLevel * speedStep;
    int yOffsetDegrees = 0;
    enum Control selected = MENU;
    char symbol = '#';
    char draft[16];
    size_t draftLength = 0;
    const char *symbolPrompt = "Choose one character";
    enum CubeColor color = DEFAULT_COLOR;
    const double radiansPerDegree = pi / 180.0;
    printf("\033[2J");
    for (;;) {
        while (_kbhit()) {
            int key = _getch();
            // Arrow keys arrive as a prefix followed by a scan code.
            if (key == 0 || key == 224) {
                int arrow = _getch();
                if (selected == Y_OFFSET) {
                    if (arrow == 72) yOffsetDegrees += 10;
                    if (arrow == 80) yOffsetDegrees -= 10;
                    yOffsetDegrees %= 360;
                } else if (selected == DIRECTION) {
                    if (arrow == 75) direction = 1;
                    if (arrow == 77) direction = -1;
                } else if (selected == SPEED) {
                    if (arrow == 72 && speedLevel < maxLevel) ++speedLevel;
                    if (arrow == 80 && speedLevel > minLevel) --speedLevel;
                }
            } else if (key == 27) selected = MENU;
            else if (selected == SYMBOL) {
                if (key == '\r') {
                    if (draftLength == 1) {
                        symbol = draft[0];
                        selected = MENU;
                    } else {
                        symbolPrompt = draftLength ? "Too many characters. Try one" :
                                                     "Empty input. Type one character";
                    }
                    draftLength = 0;
                } else if (key == '\b') {
                    if (draftLength) --draftLength;
                } else if (key >= 32 && key <= 126) {
                    if (draftLength < sizeof draft) draft[draftLength] = (char)key;
                    ++draftLength;
                }
            } else if (key == '1') selected = DIRECTION;
            else if (key == '2') selected = Y_OFFSET;
            else if (key == '3') selected = COLOR;
            else if (key == '4') selected = SPEED;
            else if (key == '5') {
                selected = SYMBOL;
                draftLength = 0;
                symbolPrompt = "Choose one character";
            }
            else if (selected == COLOR && (key == 'r' || key == 'R')) color = RED;
            else if (selected == COLOR && (key == 'c' || key == 'C')) color = CYAN;
            else if (selected == COLOR && (key == 'y' || key == 'Y')) color = YELLOW;
            else if (selected == COLOR && (key == 'w' || key == 'W')) color = DEFAULT_COLOR;
        }
        ULONGLONG now = GetTickCount64();
        double dt = fmin((now - lastTick) / 1000.0, 0.1);
        lastTick = now;
        double oldSpeed = speed;
        double targetSpeed = direction * speedLevel * speedStep;
        double maxChange = acceleration * dt;
        speed += fmax(-maxChange, fmin(targetSpeed - speed, maxChange));
        angle += (oldSpeed + speed) * 0.5 * dt;
        double yaw = angle + yOffsetDegrees * radiansPerDegree;
        // Cosine starts at the left end and gives zero travel speed at both ends.
        double centerX = (WIDTH - 1) * 0.5 - 9.0 * cos(travelPhase);
        travelPhase = fmod(travelPhase + dt * pi / halfTripSeconds, 2.0 * pi);
        char frame[HEIGHT][WIDTH];
        Point corners[8];
        memset(frame, ' ', sizeof frame);
        for (int i = 0; i < 8; ++i)
            corners[i] = project((i & 1) ? 1 : -1, (i & 2) ? 1 : -1,
                                 (i & 4) ? 1 : -1, yaw, angle * 0.6, centerX);
        for (int i = 0; i < 12; ++i)
            draw_edge(frame, corners[edges[i][0]], corners[edges[i][1]], symbol);
        printf("\033[H%s", palette[color].escape);
        for (int y = 0; y < HEIGHT; ++y)
            printf("%.*s\n", WIDTH, frame[y]);
        const char *accent = "\033[1;36m";
        const char *reset = "\033[0m";
        if (selected == SYMBOL)
            printf("\033[0m%s   Enter: apply   Esc: menu\033[K\n", symbolPrompt);
        else
            printf("\033[0m%s1. Direction%s  %s2. Y Offset%s  %s3. Color%s  %s4. Speed%s  5. Character\033[K\n",
               selected == DIRECTION ? accent : "", reset,
               selected == Y_OFFSET ? accent : "", reset,
               selected == COLOR ? accent : "", reset,
               selected == SPEED ? accent : "", reset);
        printf("%s%c%s ", accent, (now / 500) % 2 ? ' ' : '>', reset);
        switch (selected) {
            case MENU:      printf("Choose 1-5   Ctrl+C: quit"); break;
            case SYMBOL: {
                int shown = draftLength < sizeof draft ? (int)draftLength : (int)sizeof draft;
                printf("%.*s%s", shown, draft, draftLength > sizeof draft ? "..." : "");
                break;
            }
            case SPEED:     printf("4   Up / Down   Speed: %.2fx%s   Esc: menu",
                                   speedLevel * speedStep,
                                   speedLevel == minLevel ? " (min)" :
                                   speedLevel == maxLevel ? " (max)" : ""); break;
            case DIRECTION: printf("1   Left / Right   Target: %s   Esc: menu",
                                   targetSpeed > 0 ? "Left" : "Right"); break;
            case Y_OFFSET:  printf("2   Up / Down   Y offset: %+d deg   Esc: menu",
                                   yOffsetDegrees); break;
            case COLOR:     printf("[R] Red  [C] Cyan  [Y] Yellow  [W] Default | %s | Esc: menu",
                                   palette[color].name); break;
        }
        printf("\033[K");
        fflush(stdout);
        Sleep(33);
    }
}

