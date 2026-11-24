#include <sand_sim.h>

void SandSim::init(int width, int height, std::mt19937 &rand_gen) {
    this->width = width;
    this->height = height;

    left_to_right = false;

    particles = std::vector<Particle>(get_max_particles());
    updated = std::vector<bool>(get_max_particles());

    std::uniform_int_distribution<int> dist(1, 99);
    for (int i = 0; i < 200; ++i) {
        for (int j = 0; j < 150; ++j) {
            if (j % 4 == 0 && i < 35 && sin((double)i) > 0.5 == 0) {
                uint8_t r = (uint8_t)((dist(rand_gen) - 50) / 5);
                particles[i + (j * width)] = Particle {((float)i + 0.5f) / 200.0f, ((float)j + 0.5f) / 150.0f, SAND_COLOR.r + r, SAND_COLOR.g + r, SAND_COLOR.b + r, SAND_COLOR.a, ParticleType::SAND};
            } else {
                particles[i + (j * width)] = Particle {((float)i + 0.5f) / 200.0f, ((float)j + 0.5f) / 150.0f, 0, 0, 0, (char)255, ParticleType::EMPTY};
            }
        }
    }
}

void SandSim::update(std::mt19937 &rand_gen) {
    std::fill(updated.begin(), updated.end(), false);

    std::uniform_int_distribution<int> dist(1, 99);

    for (int j = 0; j < height; ++j) {
        for (int i = (left_to_right) ? 0 : 199; (left_to_right) ? i < width : i >= 0; (left_to_right) ? ++i : --i) {
            if (updated[i + (j * width)] || particles[i + (j * width)].type == ParticleType::EMPTY) {
                continue;
            }

            int index = i + (j * width);
            ParticleType p_type = particles[index].type;
            if (p_type != ParticleType::CLONER) {
                // Where the particle can move to
                MoveFlags f = can_move(i, j);

                // ----- Sand -----
                if (p_type == ParticleType::SAND && j > 0) {
                    if (f.down) {
                        swap_particles(particles[index], particles[index - width]);
                        updated[index] = true;
                        updated[index - width] = true;
                    } else if (f.down_left && f.down_right) {
                        int dir = (dist(rand_gen) < 50) ? -1 : 1;
                        swap_particles(particles[index], particles[index - width + dir]);
                        updated[index] = true;
                        updated[index - width + dir] = true;
                    } else if (f.down_left) {
                        swap_particles(particles[index], particles[index - width - 1]);
                        updated[index] = true;
                        updated[index - width - 1] = true;
                    } else if (f.down_right) {
                        swap_particles(particles[index], particles[index - width + 1]);
                        updated[index] = true;
                        updated[index - width + 1] = true;
                    }

                // ----- Water -----
                } else if (p_type == ParticleType::WATER) {
                if (f.down) {
                    swap_particles(particles[index], particles[index - width]);
                    updated[index] = true;
                    updated[index - width] = true;
                } else if (f.down_left && f.down_right) {
                    int dir = (dist(rand_gen) < 50) ? -1 : 1;
                    swap_particles(particles[index], particles[index - width + dir]);
                    updated[index] = true;
                    updated[index - width + dir] = true;
                } else if (f.down_left) {
                    swap_particles(particles[index], particles[index - width - 1]);
                    updated[index] = true;
                    updated[index - width - 1] = true;
                } else if (f.down_right) {
                    swap_particles(particles[index], particles[index - width + 1]);
                    updated[index] = true;
                    updated[index - width + 1] = true;
                } else if (f.left && f.right) {
                    int dir = (dist(rand_gen) < 50) ? -1 : 1;
                    swap_particles(particles[index], particles[index + dir]);
                    updated[index] = true;
                    updated[index + dir] = true;
                } else if (f.left) {
                    swap_particles(particles[index], particles[index - 1]);
                    updated[index] = true;
                    updated[index - 1] = true;
                } else if (f.right) {
                    swap_particles(particles[index], particles[index + 1]);
                    updated[index] = true;
                    updated[index + 1] = true;
                }
                }
            }
        }
    }

    left_to_right = !left_to_right;
}

/*
Make it check if spot has already moved!
*/
MoveFlags SandSim::can_move(int i, int j) {
    MoveFlags f = {false};

    int index = i + (j * width);

    bool has_below = (j > 0), has_left = (i > 0), has_right = (i < width - 1);
    
    switch (particles[index].type) {
        case ParticleType::SAND:
            if (has_below) {
                f.down = (particles[index - width].type == ParticleType::EMPTY || particles[index - width].type == ParticleType::WATER);
                if (has_left) {
                    f.down_left = (particles[index - width - 1].type == ParticleType::EMPTY || particles[index - width - 1].type == ParticleType::WATER);
                }
                if (has_right) {
                    f.down_right = (particles[index - width + 1].type == ParticleType::EMPTY || particles[index - width + 1].type == ParticleType::WATER);
                }
            }
            break;
            
        case ParticleType::WATER:
            if (has_below) {
                f.down = (particles[index - width].type == ParticleType::EMPTY);
                if (has_left) {
                    f.down_left = (particles[index - width - 1].type == ParticleType::EMPTY);
                }
                if (has_right) {
                    f.down_right = (particles[index - width + 1].type == ParticleType::EMPTY);
                }
            }
            if (has_left) {
                f.left = (particles[index - 1].type == ParticleType::EMPTY);
            }
            if (has_right) {
                f.right = (particles[index + 1].type == ParticleType::EMPTY);
            }
            break;

        default:
            break;
    }

    return f;
}

void SandSim::add_particles(ParticleType type, char r, char g, char b, char a, int x, int y, int radius) {
    if (particles[x + (y * width)].type == ParticleType::EMPTY) {
        particles[x + (y * width)].r = r;
        particles[x + (y * width)].g = g;
        particles[x + (y * width)].b = b;
        particles[x + (y * width)].a = a;
        particles[x + (y * width)].type = type;
    }
}

const std::vector<Particle>& SandSim::get_particles() const {
    return particles;
}

int SandSim::get_max_particles() {
    return width * height;
}