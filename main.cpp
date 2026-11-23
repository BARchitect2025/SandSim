#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstddef>
#include <random>

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

enum class ParticleType {
    SAND,
    WATER,
    EMPTY
};

struct Color {
    uint8_t r, g, b, a;
};

const Color SAND_COLOR = {204, 188, 138, 255};
const Color WATER_COLOR = {14, 135, 204, 255};

struct Particle {
    float x, y;
    uint8_t r, g, b, a;

    ParticleType type;
};

bool mouse_button_pressed = false;

int WIDTH = 800, HEIGHT = 600;

float delta = 0.0f;
float last_frame = 0.0f;

int mouse_pos_pixels_x = 0, mouse_pos_pixels_y = 0;

ParticleType selected_type = ParticleType::SAND;
int selected_type_num = 0;

GLuint compile_shaders(const char *vs_source, const char *fs_source);
void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void swap_particles(Particle &a, Particle &b);

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<>dist(1, 99);

    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const int MAX_PARTICLES = 200 * 150;

    Particle particles[200][150];

    for (int i = 0; i < 200; ++i) {
        for (int j = 0; j < 150; ++j) {
            if (j % 4 == 0 && i < 35 && sin((double)i) > 0.5 == 0) {
                uint8_t r = (uint8_t)((dist(gen) - 50) / 5);
                particles[i][j] = Particle {((float)i + 0.5f) / 200.0f, ((float)j + 0.5f) / 150.0f, SAND_COLOR.r + r, SAND_COLOR.g + r, SAND_COLOR.b + r, SAND_COLOR.a, ParticleType::SAND};
            } else {
                particles[i][j] = Particle {((float)i + 0.5f) / 200.0f, ((float)j + 0.5f) / 150.0f, 0, 0, 0, 255, ParticleType::EMPTY};
            }
        }
    }
    
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Sand", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);

    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    GLuint program = compile_shaders(vs_source, fs_source);

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(Particle), NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, x));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Particle), (void*)offsetof(Particle, r));

    glPointSize(4.0f);

    float accumulator = 0.0f;
    const float TIMESTEP = 1.0f / 60.0f;

    while (!glfwWindowShouldClose(window)) {
        float current_frame = static_cast<float>(glfwGetTime());
        delta = current_frame - last_frame;
        last_frame = current_frame;
        accumulator += delta;

        while (accumulator > TIMESTEP) {
            accumulator -= TIMESTEP;

            if (mouse_button_pressed) {
                int gx = mouse_pos_pixels_x / 4;
                int gy = mouse_pos_pixels_y / 4;

                if (gx >= 0 && gx < 200 && gy >= 0 && gy < 150) {
                    if (particles[gx][gy].type == ParticleType::EMPTY) {
                        int r = (int)((dist(gen) - 50) / 5);
                        if (selected_type == ParticleType::SAND) {
                            particles[gx][gy].r = SAND_COLOR.r + r;
                            particles[gx][gy].g = SAND_COLOR.g + r;
                            particles[gx][gy].b = SAND_COLOR.b + r;
                            particles[gx][gy].a = SAND_COLOR.a;
                        } else if (selected_type == ParticleType::WATER) {
                            particles[gx][gy].r = WATER_COLOR.r + r;
                            particles[gx][gy].g = WATER_COLOR.g + r;
                            particles[gx][gy].b = WATER_COLOR.b + r;
                            particles[gx][gy].a = WATER_COLOR.a;
                        }
                        particles[gx][gy].type = selected_type;
                    }
                }
            }

            bool updated[200][150] = {false};

            for (int j = 0; j < 150; ++j) {
                for (int i = 0; i < 200; ++i) {
                    if (updated[i][j] || particles[i][j].type == ParticleType::EMPTY) {
                        continue;
                    }

                    if (particles[i][j].type == ParticleType::SAND && j > 0) {
                        if (particles[i][j - 1].type == ParticleType::EMPTY) {
                            swap_particles(particles[i][j], particles[i][j - 1]);
                            updated[i][j] = true;
                            updated[i][j - 1] = true;
                        } else if (i > 0 && particles[i - 1][j - 1].type == ParticleType::EMPTY) {
                            swap_particles(particles[i][j], particles[i - 1][j - 1]);
                            updated[i][j] = true;
                            updated[i - 1][j - 1] = true;
                        } else if (i < 199 && particles[i + 1][j - 1].type == ParticleType::EMPTY) {
                            swap_particles(particles[i][j], particles[i + 1][j - 1]);
                            updated[i][j] = true;
                            updated[i + 1][j - 1] = true;
                        } else {
                            bool down_left = (i > 0 && particles[i - 1][j - 1].type == ParticleType::WATER);
                            bool down_right = (i < 199 && particles[i + 1][j - 1].type == ParticleType::WATER);

                            if (particles[i][j - 1].type == ParticleType::WATER) {
                                swap_particles(particles[i][j], particles[i][j - 1]);
                                updated[i][j] = true;
                                updated[i][j - 1] = true;
                            } else if (down_left && down_right) {
                                int dir = (dist(gen) < 50) ? -1 : 1;
                                swap_particles(particles[i][j], particles[i + dir][j - 1]);
                                updated[i][j] = true;
                                updated[i + dir][j - 1] = true;
                            } else if (down_left) {
                                swap_particles(particles[i][j], particles[i - 1][j - 1]);
                                updated[i][j] = true;
                                updated[i - 1][j - 1] = true;
                            } else if (down_right) {
                                swap_particles(particles[i][j], particles[i + 1][j - 1]);
                                updated[i][j] = true;
                                updated[i + 1][j - 1] = true;
                            }
                        }
                    } else if (particles[i][j].type == ParticleType::WATER) {
                        bool moved = false;

                        if (j > 0) {
                            // Down
                            if (particles[i][j - 1].type == ParticleType::EMPTY) {
                                swap_particles(particles[i][j], particles[i][j - 1]);
                                updated[i][j] = true;
                                updated[i][j - 1] = true;
                                moved = true;
                            } else {
                                bool can_diag_l = (i > 0 && particles[i - 1][j - 1].type == ParticleType::EMPTY);
                                bool can_diag_r = (i < 199 && particles[i + 1][j - 1].type == ParticleType::EMPTY);

                                if (can_diag_l && can_diag_r) {
                                    int dir = (dist(gen) < 50) ? -1 : 1;
                                    swap_particles(particles[i][j], particles[i + dir][j - 1]);
                                    updated[i][j] = true;
                                    updated[i + dir][j - 1] = true;
                                    moved = true;
                                } else if (can_diag_l) {
                                    swap_particles(particles[i][j], particles[i - 1][j - 1]);
                                    updated[i][j] = true;
                                    updated[i - 1][j - 1] = true;
                                    moved = true;
                                } else if (can_diag_r) {
                                    swap_particles(particles[i][j], particles[i + 1][j - 1]);
                                    updated[i][j] = true;
                                    updated[i + 1][j - 1] = true;
                                    moved = true;
                                }
                            }
                        }

                        if (!moved) {
                            bool can_left = (i > 0 && particles[i - 1][j].type == ParticleType::EMPTY);
                            bool can_right = (i < 199 && particles[i + 1][j].type == ParticleType::EMPTY);

                            if (can_left && can_right) {
                                int dir = (dist(gen) < 50) ? -1 : 1;
                                swap_particles(particles[i][j], particles[i + dir][j]);
                                updated[i][j] = true;
                                updated[i + dir][j] = true;
                            } else if (can_left) {
                                swap_particles(particles[i][j], particles[i - 1][j]);
                                updated[i][j] = true;
                                updated[i - 1][j] = true;
                            } else if (can_right) {
                                swap_particles(particles[i][j], particles[i + 1][j]);
                                updated[i][j] = true;
                                updated[i + 1][j] = true;
                            }
                        }
                    }
                }
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // Assign particle data
        glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(Particle), NULL, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_PARTICLES * sizeof(Particle), particles);

        // Draw particles
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);

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
    mouse_pos_pixels_y = HEIGHT - 1 - (int)ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse_button_pressed = true;
    } else {
        mouse_button_pressed = false;
    }
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    selected_type_num += (int)yoffset;

    if (selected_type_num > 1) {
        selected_type_num = 0;
    } else if (selected_type_num < 0) {
        selected_type_num = 1;
    }

    switch (selected_type_num) {
        case 0: selected_type = ParticleType::SAND; break;
        case 1: selected_type = ParticleType::WATER; break;
    }
}

void swap_particles(Particle &a, Particle &b) {
    std::swap(a.r, b.r);
    std::swap(a.g, b.g);
    std::swap(a.b, b.b);
    std::swap(a.a, b.a);
    std::swap(a.type, b.type);
}