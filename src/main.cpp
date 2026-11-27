#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstddef>
#include <random>

#include <sand_sim.h>

const char* vs_source = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"layout (location = 1) in vec4 aColor;\n"
"out vec4 vColor;\n"
"void main(void)\n"
"{\n"
"gl_Position = vec4(aPos * 2.0 - 1.0, 0.0, 1.0);\n"
"vColor = aColor;\n"
"}\n";

const char* fs_source = "#version 330 core\n"
"in vec4 vColor;\n"
"out vec4 color;\n"
"void main(void)\n"
"{\n"
"color = vColor;\n"
"}\n";

bool mouse_button_pressed = false;
bool paused = false;
bool shifted = false;

int add_radius = 2;

int width = 800, height = 600;

int tick_rate = 60;
double TIMESTEP = 1.0f / tick_rate;
int ticks = 0;

double delta = 0.0f;
double last_frame = 0.0f;

int mouse_pos_pixels_x = 0, mouse_pos_pixels_y = 0;

ParticleType selected_type = ParticleType::SAND;
int selected_type_num = 0;

GLuint compile_shaders(const char *vs_source, const char *fs_source);
void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

int main() {
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(width, height, "Sand", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Sand simulation initialization
    SandSim sim;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> dist(1, 99);

    sim.init(width / 4, height / 4, gen);

    // OpenGL shader compilation
    GLuint program = compile_shaders(vs_source, fs_source);

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sim.get_max_particles() * sizeof(Particle), NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, x));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Particle), (void*)offsetof(Particle, r));

    glPointSize(4.0f);

    double accumulator = 0.0f;

    std::vector<float> deltas;

    while (!glfwWindowShouldClose(window)) {
        int max_particles = sim.get_max_particles();

        double current_frame = static_cast<float>(glfwGetTime());
        delta = current_frame - last_frame;
        last_frame = current_frame;
        accumulator += delta;

        deltas.push_back(delta);

        if (mouse_button_pressed) {
            int gx = mouse_pos_pixels_x / 4;
            int gy = mouse_pos_pixels_y / 4;

            if (gx >= 0 && gx < 200 && gy >= 0 && gy < 150) {
                int r = (int)((dist(gen) - 50) / 5);
                if (selected_type == ParticleType::SAND) {
                    sim.add_particles(selected_type, SAND_COLOR.r + r, SAND_COLOR.g + r, SAND_COLOR.b + r, SAND_COLOR.a, gx, gy, add_radius);
                } else if (selected_type == ParticleType::WATER) {
                    sim.add_particles(selected_type, WATER_COLOR.r + r, WATER_COLOR.g + r, WATER_COLOR.b + r, WATER_COLOR.a, gx, gy, add_radius);
                }
            }
        }

        while (accumulator > TIMESTEP) {
            accumulator -= TIMESTEP;

            if (!paused) {
                sim.update(gen);
            }

            ++ticks;

            if (ticks >= tick_rate) {
                float total = 0.0f;
                for (int i = 0; i < deltas.size(); ++i) {
                    total += deltas[i];
                }
                std::cout << "FPS: " << (int)(deltas.size() / total + 0.5f) << std::endl;
                deltas = std::vector<float>();
                ticks = 0;
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // Assign particle data
        glBufferData(GL_ARRAY_BUFFER, max_particles * sizeof(Particle), NULL, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, max_particles * sizeof(Particle), sim.get_particles().data());

        // Draw particles
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, max_particles);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
    
    glfwTerminate();
    return 0;
}

GLuint compile_shaders(const char *vs_source, const char *fs_source) {
    GLuint vs;
    GLuint fs;
    GLuint program;

    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_source, NULL);
    glCompileShader(vs);

    fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_source, NULL);
    glCompileShader(fs);

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    mouse_pos_pixels_x = (int)xpos;
    mouse_pos_pixels_y = height - 1 - (int)ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse_button_pressed = true;
    } else {
        mouse_button_pressed = false;
    }
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (!shifted) {
        selected_type_num += (int)yoffset;
        if (selected_type_num > 2) {
            selected_type_num = 0;void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        } else if (selected_type_num < 0) {
            selected_type_num = 2;
        }
    } else {
        if (yoffset < 0.0 && add_radius <= 0) {
            // Do nothing
        } else if (yoffset > 0.0 && add_radius >= 12) {
            // Do nothing
        } else {
            add_radius += (int)yoffset;
        }
    }

    switch (selected_type_num) {
        case 0: selected_type = ParticleType::SAND; break;
        case 1: selected_type = ParticleType::WATER; break;
        case 2: selected_type = ParticleType::CLONER; break;
        default: break;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        paused = !paused;
    } else if (key == GLFW_KEY_UP && action == GLFW_PRESS && tick_rate < 240.0f) {
        tick_rate += 60;
        TIMESTEP = 1.0f / tick_rate;
    } else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS && tick_rate > 60.0f) {
        tick_rate -= 60;
        TIMESTEP = 1.0f / (float)tick_rate;
    } else if (key == GLFW_KEY_LEFT_SHIFT) {
        if (action == GLFW_PRESS) {
            shifted = true;
        } else if (action == GLFW_RELEASE) {
            shifted = false;
        }
    }
}