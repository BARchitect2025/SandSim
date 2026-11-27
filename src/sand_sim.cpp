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
                uint8_t r = static_cast<uint8_t>((dist(rand_gen) - 50) / 5);
                particles[i + (j * width)] = Particle {((float)i + 0.5f) / 200.0f, ((float)j + 0.5f) / 150.0f, static_cast<uint8_t>(SAND_COLOR.r + r), static_cast<uint8_t>(SAND_COLOR.g + r), static_cast<uint8_t>(SAND_COLOR.b + r), SAND_COLOR.a, ParticleType::SAND};
            } else {
                particles[i + (j * width)] = Particle {((float)i + 0.5f) / 200.0f, ((float)j + 0.5f) / 150.0f, 0, 0, 0, static_cast<uint8_t>(255), ParticleType::EMPTY};
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
                // Particle move flags
                uint16_t f = can_move(i, j);

                // ----- Sand Lambda -----
                auto move_sand_to = [&](int target_x, int target_y) {
                    int target_index = target_x + (target_y * width);

                    if (particles[target_index].type == ParticleType::WATER) {
                        bool can_up_left = (target_y + 1 < height) && (target_x - 1 >= 0) &&(particles[(target_x - 1) + ((target_y + 1) * width)].type == ParticleType::EMPTY);
                        bool can_up_right = (target_y + 1 < height) && (target_x + 1 < width) && (particles[(target_x + 1) + ((target_y + 1) * width)].type == ParticleType::EMPTY);

                        if (can_up_left || can_up_right) {
                            int dir = 0;
                            if (can_up_left && can_up_right) {
                                dir = (dist(rand_gen) < 50) ? -1 : 1;
                            } else {
                                dir = can_up_left ? -1 : 1;
                            }

                            int water_dest_index = (target_x + dir) + ((target_y + 1) * width);

                            swap_particles(particles[target_index], particles[water_dest_index]);
                            updated[water_dest_index] = true;
                        }
                    }

                    swap_particles(particles[index], particles[target_index]);
                    updated[index] = true;
                    updated[target_index] = true;
                };

                // ----- Sand -----
                if (p_type == ParticleType::SAND && j > 0) {
                    // Down
                    if (f & 0x0004) {
                        move_sand_to(i, j - 1);

                    // Random between Down Left and Down Right
                    } else if ((f & 0x0002) && (f & 0x0008)) {
                        int dir = (dist(rand_gen) < 50) ? -1 : 1;
                        move_sand_to(i + dir, j - 1);

                    // Down Left
                    } else if (f & 0x0002) {
                        move_sand_to(i - 1, j - 1);

                    // Down Right
                    } else if (f & 0x0008) {
                        move_sand_to(i + 1, j - 1);
                    }
                // ----- Water -----
                } else if (p_type == ParticleType::WATER) {
                    uint8_t e_f = (f & 0x00ff) & ~(f >> 8);
                    if (e_f & 0x04) {
                        swap_particles(particles[index], particles[index - width]);
                        updated[index] = true;
                        updated[index - width] = true;
                    } else if ((e_f & 0x02) && (e_f & 0x08)) {
                        int dir = (dist(rand_gen) < 50) ? -1 : 1;
                        swap_particles(particles[index], particles[index - width + dir]);
                        updated[index] = true;
                        updated[index - width + dir] = true;
                    } else if (e_f & 0x02) {
                        swap_particles(particles[index], particles[index - width - 1]);
                        updated[index] = true;
                        updated[index - width - 1] = true;
                    } else if (e_f & 0x08) {
                        swap_particles(particles[index], particles[index - width + 1]);
                        updated[index] = true;
                        updated[index - width + 1] = true;
                    } else if ((e_f & 0x01) && (e_f & 0x10)) {
                        int dir = (dist(rand_gen) < 50) ? -1 : 1;
                        swap_particles(particles[index], particles[index + dir]);
                        updated[index] = true;
                        updated[index + dir] = true;
                    } else if (e_f & 0x01) {
                        swap_particles(particles[index], particles[index - 1]);
                        updated[index] = true;
                        updated[index - 1] = true;
                    } else if (e_f & 0x10) {
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
    Move check helper function

    Bit 0: Left
    Bit 1: Down Left
    Bit 2: Down
    Bit 3: Down Right
    Bit 4: Right
    Bit 5: Up Right
    Bit 6: Up
    Bit 7: Up Left

    Next 8 Bits are same, but say if that particle is water

    Example for me: 0b0010'0100'1100'0100
    Would be:
    - Down
    - Up
    - Up Left
    - Water Down
    - Water Up Right
*/
uint16_t SandSim::can_move(int i, int j) {
    uint16_t flags = 0;

    int index = i + (j * width);

    bool has_below = (j > 0), has_left = (i > 0), has_right = (i < width - 1), has_above = (j < 149);
    
    if (has_below && has_above) {
        // Down
        if (particles[index - width].type == ParticleType::EMPTY) {
            flags |= 0x0004;
        } else if (particles[index - width].type == ParticleType::WATER) {
            flags |= 0x0404;
        }

        // Left
        if (has_left) {
            ParticleType t1 = particles[index - width - 1].type,
                         t2 = particles[index - 1].type,
                         t3 = particles[index + width - 1].type;
            
            // Down left
            if (t1 == ParticleType::EMPTY) {
                flags |= 0x0002;
            } else if (t1 == ParticleType::WATER) {
                flags |= 0x0202;
            }

            // Left
            if (t2 == ParticleType::EMPTY) {
                flags |= 0x0001;
            } else if (t2 == ParticleType::WATER) {
                flags |= 0x0101;
            }

            // Up left
            if (t3 == ParticleType::EMPTY) {
                flags |= 0x0080;
            } else if (t3 == ParticleType::WATER) {
                flags |= 0x8080;
            }
        }

        // Right
        if (has_right) {
            ParticleType t1 = particles[index - width + 1].type,
                         t2 = particles[index + 1].type,
                         t3 = particles[index + width + 1].type;
            
            // Down right
            if (t1 == ParticleType::EMPTY) {
                flags |= 0x0008;
            } else if (t1 == ParticleType::WATER) {
                flags |= 0x0808;
            }

            // Right
            if (t2 == ParticleType::EMPTY) {
                flags |= 0x0010;
            } else if (t2 == ParticleType::WATER) {
                flags |= 0x1010;
            }

            // Up right
            if (t3 == ParticleType::EMPTY) {
                flags |= 0x0020;
            } else if (t3 == ParticleType::WATER) {
                flags |= 0x2020;
            }
        }

        if (particles[index + width].type == ParticleType::EMPTY) {
            flags |= 0x0040;
        } else if (particles[index + width].type == ParticleType::WATER) {
            flags |= 0x4040;
        }
    } else if (has_below) {
        // Down
        if (particles[index - width].type == ParticleType::EMPTY) {
            flags |= 0x0004;
        } else if (particles[index - width].type == ParticleType::WATER) {
            flags |= 0x0404;
        }

        // Left
        if (has_left) {
            ParticleType t1 = particles[index - width - 1].type,
                         t2 = particles[index - 1].type;
            
            // Down left
            if (t1 == ParticleType::EMPTY) {
                flags |= 0x0002;
            } else if (t1 == ParticleType::WATER) {
                flags |= 0x0202;
            }

            // Left
            if (t2 == ParticleType::EMPTY) {
                flags |= 0x0001;
            } else if (t2 == ParticleType::WATER) {
                flags |= 0x0101;
            }
        }

        // Right
        if (has_right) {
            ParticleType t1 = particles[index - width + 1].type,
                         t2 = particles[index + 1].type;
            
            // Down right
            if (t1 == ParticleType::EMPTY) {
                flags |= 0x0008;
            } else if (t1 == ParticleType::WATER) {
                flags |= 0x0808;
            }

            // Right
            if (t2 == ParticleType::EMPTY) {
                flags |= 0x0010;
            } else if (t2 == ParticleType::WATER) {
                flags |= 0x1010;
            }
        }
    } else if (has_above) {
        // Left
        if (has_left) {
            ParticleType t1 = particles[index - 1].type,
                         t2 = particles[index + width - 1].type;

            // Left
            if (t1 == ParticleType::EMPTY) {
                flags |= 0x0001;
            } else if (t1 == ParticleType::WATER) {
                flags |= 0x0101;
            }

            // Up left
            if (t2 == ParticleType::EMPTY) {
                flags |= 0x0080;
            } else if (t2 == ParticleType::WATER) {
                flags |= 0x8080;
            }
        }

        // Right
        if (has_right) {
            ParticleType t1 = particles[index + 1].type,
                         t2 = particles[index + width + 1].type;

            // Right
            if (t1 == ParticleType::EMPTY) {
                flags |= 0x0010;
            } else if (t1 == ParticleType::WATER) {
                flags |= 0x1010;
            }

            // Up right
            if (t2 == ParticleType::EMPTY) {
                flags |= 0x0020;
            } else if (t2 == ParticleType::WATER) {
                flags |= 0x2020;
            }
        }

        if (particles[index + width].type == ParticleType::EMPTY) {
            flags |= 0x0040;
        } else if (particles[index + width].type == ParticleType::WATER) {
            flags |= 0x4040;
        }
    }

    return flags;
}

void SandSim::add_particles(ParticleType type, char r, char g, char b, char a, int x, int y, int radius) {
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            int x1 = x + dx;
            int y1 = y + dy;

            if (x1 < 0 || x1 >= width || y1 < 0 || y1 > height) {
                continue;
            }

            int index = x1 + (y1 * width);
            if (particles[index].type == ParticleType::EMPTY) {
                particles[index].r = r;
                particles[index].g = g;
                particles[index].b = b;
                particles[index].a = a;
                particles[index].type = type;
            }
        }
    }
}

const std::vector<Particle>& SandSim::get_particles() const {
    return particles;
}

int SandSim::get_max_particles() {
    return width * height;
}