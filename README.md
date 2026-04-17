# GCalc - Graphing Calculator

A real-time graphing calculator built in C with [raylib](https://www.raylib.com/).
Plot mathematical expressions in `x` with smooth zoom and pan.

> raylib is not included. It is downloaded automatically on first build.

## Controls

| Input | Action |
|---|---|
| Scroll wheel | Zoom in or out, centered on the cursor |
| Right / Middle drag | Pan the graph |
| Textbox row | Edit a function with raygui textboxes |
| Top-left theme button | Toggle light and dark mode |
| Top-left thickness slider | Change line thickness for all functions |
| `+` / `-` | Add or remove function rows |
| F11 | Toggle fullscreen |
| ESC | Quit |

Expressions are edited in the bottom-left control panel and update the graph in real time.

## Getting Started

### 1. Clone

```bash
git clone https://github.com/EAdler98/GCalc.git
cd GCalc
```

### 2. Build

Pick the instructions for your platform.

#### Windows - Visual Studio 2022

Run the batch file, then open the generated solution:

```bat
build-VisualStudio2022.bat
```

Open `GCalc.vcxproj` in Visual Studio, set the startup project to `GCalc`, and press `F5`.

#### Windows - MinGW-W64 (GCC)

Requires [MinGW-W64](https://www.mingw-w64.org/) on your `PATH`.

```bat
build-MinGW-W64.bat
mingw32-make
```

#### Linux / WSL2

Install the X11 development headers once:

```bash
sudo apt-get install -y \
  libx11-dev libxrandr-dev libxinerama-dev \
  libxcursor-dev libxi-dev libgl1-mesa-dev
```

Generate makefiles and build:

```bash
cd build && ./premake5 gmake2 && cd ..
make
```

The binary is placed in `bin/Debug/GCalc.exe` or `bin/Release/GCalc.exe` on Windows.

## Architecture

| File | Responsibility |
|---|---|
| `src/main.c` | Window setup, camera init, frame loop |
| `src/graph.c/h` | Camera update, grid, axes, and function rendering |
| `src/ui.c/h` | raygui-based function panel controls |
| `src/parser.c/h` | Infix to postfix parsing and evaluation |
| `src/token.c/h` | Tokenization and implicit multiplication rules |
| `src/stack.c/h` | Token stack used by the parser |

## Supported Syntax

```text
x^2 + 2*x - 3
2x(x+1)
(x-1)(x+1)
-x^2 + 3
1/x
sqrt(x + 1)
log(x)
sin(x)
cos(x)
```

Operators: `+` `-` `*` `/` `^`

Notes:
- Only lowercase `x` is accepted as a variable.
- Adjacent variables like `xx` are rejected; write `x*x` explicitly.
- Implicit multiplication works for `2x`, `2(x+1)`, and `(x-1)(x+1)`.
- Supported functions are `sqrt`, `log` (natural log), `sin`, and `cos`.

## Tests

Parser unit tests live in `tests/parser_tests.c` and do not depend on raylib rendering.

```bash
make -f GCalc.make test_runner
./test_runner
```

GitHub Actions runs the parser tests on every push and pull request to `main`.
