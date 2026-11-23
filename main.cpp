#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstddef>

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

int WIDTH = 800, HEIGHT = 600;

float delta = 0.0f;
float last_frame = 0.0f;

int mouse_pos_pixels_x = 0, mouse_pos_pixels_y = 0;

enum class ParticleType {
    SAND,
    WATER,
    EMPTY
};

struct Particle {
    float x, y;
    uint8_t r, g, b, a;

    ParticleType type;
};

GLuint compileShaders(const char *vs_source, const char *fs_source);
void mouse_pos_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

int main() {
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const int MAX_PARTICLES = 200 * 150;

    Particle particles[200][150];

    for (int i = 0; i < 200; ++i) {
        for (int j = 0; j < 150; ++j) {
            if (j % 4 == 0 && i < 25) {
                particles[i][j] = Particle {(float)i / 200.0f, (float)j / 150.0f, 204, 188, 138, 255, ParticleType::SAND};
            } else {
                particles[i][j] = Particle {(float)i / 200.0f, (float)j / 150.0f, 0, 0, 0, 255, ParticleType::EMPTY};
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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    GLuint program = compileShaders(vs_source, fs_source);

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

    while (!glfwWindowShouldClose(window)) {
        float current_frame = static_cast<float>(glfwGetTime());
        delta = current_frame - last_frame;
        last_frame = current_frame;

        if (mouse_button_pressed) {
            int gx = mouse_pos_pixels_x / 4;
            int gy = mouse_pos_pixels_y / 4;

            if (gx >= 0 && gx < 200 && gy >= 0 && gy < 150) {
                if (particles[gx][gy].type == ParticleType::EMPTY) {
                    particles[gx][gy].r = 204;
                    particles[gx][gy].g = 188;
                    particles[gx][gy].b = 138;
                    particles[gx][gy].a = 255;
                    particles[gx][gy].type = ParticleType::SAND;
                }
            }
        }

        for (int i = 0; i < 200; ++i) {
            for (int j = 1; j < 150; ++j) {
                if (particles[i][j].type == ParticleType::SAND) {
                    if (particles[i][j - 1].type == ParticleType::EMPTY) {
                        particles[i][j - 1].r = particles[i][j].r;
                        particles[i][j - 1].g = particles[i][j].g;
                        particles[i][j - 1].b = particles[i][j].b;
                        particles[i][j - 1].a = particles[i][j].a;
                        particles[i][j - 1].type = particles[i][j].type;

                        particles[i][j].r = 0;
                        particles[i][j].g = 0;
                        particles[i][j].b = 0;
                        particles[i][j].a = 255;
                        particles[i][j].type = ParticleType::EMPTY;
                    } else if (i > 0 && particles[i - 1][j - 1].type == ParticleType::EMPTY) {
                        particles[i - 1][j - 1].r = particles[i][j].r;
                        particles[i - 1][j - 1].g = particles[i][j].g;
                        particles[i - 1][j - 1].b = particles[i][j].b;
                        particles[i - 1][j - 1].a = particles[i][j].a;
                        particles[i - 1][j - 1].type = particles[i][j].type;

                        particles[i][j].r = 0;
                        particles[i][j].g = 0;
                        particles[i][j].b = 0;
                        particles[i][j].a = 255;
                        particles[i][j].type = ParticleType::EMPTY;
                    } else if (i < 199 && particles[i + 1][j - 1].type == ParticleType::EMPTY) {
                        particles[i + 1][j - 1].r = particles[i][j].r;
                        particles[i + 1][j - 1].g = particles[i][j].g;
                        particles[i + 1][j - 1].b = particles[i][j].b;
                        particles[i + 1][j - 1].a = particles[i][j].a;
                        particles[i + 1][j - 1].type = particles[i][j].type;

                        particles[i][j].r = 0;
                        particles[i][j].g = 0;
                        particles[i][j].b = 0;
                        particles[i][j].a = 255;
                        particles[i][j].type = ParticleType::EMPTY;
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

GLuint compileShaders(const char *vs_source, const char *fs_source) {
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
