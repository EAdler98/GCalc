# GCalc — Graphing Calculator

A real-time graphing calculator built in C with [raylib](https://www.raylib.com/).  
Plot any mathematical expression in `x` with smooth zoom and pan.

> raylib is **not included** — it is downloaded automatically on first build.

## Controls

| Input | Action |
|---|---|
| Scroll wheel | Zoom in / out (centered on cursor) |
| Right / Middle drag | Pan |
| ESC | Quit |

---

## Getting Started

### 1. Clone

```bash
git clone https://github.com/your-username/GCalc.git
cd GCalc
```

### 2. Build

Pick the instructions for your platform:

---

#### Windows — Visual Studio 2022

Run the batch file, then open the generated solution:

```bat
build-VisualStudio2022.bat
```

Open `GCalc.vcxproj` in Visual Studio, set the startup project to **GCalc**, and press **F5**.

---

#### Windows — MinGW-W64 (GCC)

Requires [MinGW-W64](https://www.mingw-w64.org/) on your `PATH`.

```bat
build-MinGW-W64.bat
mingw32-make
```

---

#### Linux / WSL2

Install the X11 development headers (one time):

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

---

The binary is placed in `bin/Debug/GCalc` (or `bin/Release/GCalc`).

---

## Architecture

| File | Responsibility |
|---|---|
| `src/main.c` | Window, camera init, main loop |
| `src/graph.c/h` | Camera update, grid/axes/function rendering |
| `src/parser.c/h` | Shunting-yard infix → postfix + evaluator |
| `src/token.c/h` | Lexer — tokenizes input strings |
| `src/stack.c/h` | Token stack used by the parser |

## Supported Syntax

```
x^2 + 2*x - 3
2x(x+1)          # implicit multiplication
(x-1)(x+1)
```

Operators: `+`  `-`  `*`  `/`  `^`
