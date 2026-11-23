# SandSim

This is a sand simulation written in C++ with OpenGL.

## Running

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
g++ main.cpp glad.c -Iinclude -o main -lGL -lglfw -ldl
```

#### MacOS (Homebrew):
```bash
g++ main.cpp glad.c -Iinclude -I/opt/homebrew/include -L/opt/homebrew/lib -o main -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
```

#### Windows:
* **MinGW / MSYS2:**
```bash
g++ main.cpp glad.c -Iinclude -Llib -o main.exe -lglfw3 -lopengl32 -lgdi32 -luser32 -lkernel32
```

* **MSVC**
```cmd
cl main.cpp glad.c /I include /Fe:main.exe /link /LIBPATH:lib glfw3.lib opengl32.lib gdi32.lib user32.lib shell32.lib
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