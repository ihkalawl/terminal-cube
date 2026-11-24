# terminal-cube
An interactive 3D cube written in C. Change its spin direction, angle, color, speed, and drawing character while it moves across your terminal.

# Rotating Cube

A wireframe cube spins in your terminal and glides from side to side. Use the numbered menu to change it while it moves.

## Build and run

Open a terminal in the folder containing `cube.c`. Use a window at least 80 columns wide and 24 rows tall.

### Linux and macOS

Use GCC or Clang. On macOS, the Xcode Command Line Tools provide `cc`. On Linux, install your distribution's C compiler and development headers if needed.

```sh
cc -std=c11 -O2 -Wall -Wextra cube.c -o cube -lm
./cube
```

### Windows

Use GCC from MinGW-w64 and run these commands in PowerShell or Windows Terminal.

```powershell
gcc -std=c11 -O2 -Wall -Wextra cube.c -o cube.exe -lm
.\cube.exe
```

## Controls

Press a menu number to select a control. Press **Esc** to return to the menu or **Ctrl+C** to quit.

| Option | Keys | Effect |
| --- | --- | --- |
| 1. Direction | Left / Right | Choose the spin direction. The cube eases through reversals. |
| 2. Y Offset | Up / Down | Adjust rotation around the Y axis by 10 degrees. |
| 3. Color | R / C / Y / W | Choose red, cyan, yellow, or the terminal's default text color. |
| 4. Speed | Up / Down | Adjust spin speed from 0.25× to 2× in 0.25× steps. |
| 5. Character | Type one printable ASCII character, then Enter | Replace the wireframe character. Backspace edits and Esc cancels. |
