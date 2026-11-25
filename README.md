# SandSim

This is a sand simulation written in C++ with OpenGL.

## Running

### 1. Download
Download from [Releases](https://github.com/barchitect2025/sandsim/releases).

### 2. Run
```bash
# Linux / MacOS
./main

# Windows
main.exe
```

## Compiling from source

### 1. Clone the repository
```bash
git clone https://github.com/barchitect2025/sandsim.git
cd sandsim
```

### 2. Dependencies
- GLFW

### 3. Compile

#### Linux:
```bash
g++ src/main.cpp src/particle.cpp src/sand_sim.cpp src/glad.c -Iinclude -o sandsim -lGL -lglfw -ldl
```

#### MacOS (Homebrew):
```bash
g++ src/main.cpp src/particle.cpp src/sand_sim.cpp src/glad.c -Iinclude -I/opt/homebrew/include -L/opt/homebrew/lib -o sandsim -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
```

#### Windows:
* **MinGW / MSYS2:**
```bash
g++ src/main.cpp src/particle.cpp src/sand_sim.cpp src/glad.c -Iinclude -Llib -o sandsim.exe -lglfw3 -lopengl32 -lgdi32 -luser32 -lkernel32
```

* **MSVC**
```cmd
cl src/main.cpp src/particle.cpp src/sand_sim.cpp src/glad.c /I include /Fe:sandsim.exe /link /LIBPATH:lib glfw3.lib opengl32.lib gdi32.lib user32.lib shell32.lib
```

### 4. Run
```bash
# Linux / MacOS
./main

# Windows
main.exe
```

## Controls

| Key | Action |
| :--- | :--- |
| **Left Click** | Add selected element |
| **Right Click** | Erase cell (YET TO BE ADDED) |
| **Scroll Wheel** | Change element |
| **Shift + Scroll Wheel** | Change brush size (YET TO BE ADDED) |