#ifndef PARTICLE_H_
#define PARTICLE_H_

enum class ParticleType {
    SAND,
    WATER,
    CLONER,
    EMPTY
};

struct Color {
    char r, g, b, a;
};

const Color SAND_COLOR = {(char)204, (char)188, (char)138, (char)255};
const Color WATER_COLOR = {(char)14, (char)135, (char)204, (char)255};

struct MoveFlags {
    bool down, down_left, down_right, left, right;
};

struct Particle {
    float x, y;
    char r, g, b, a;

    ParticleType type;
};

struct Cloner : Particle {
    ParticleType cloning;
};

void swap_particles(Particle &a, Particle &b);

#endif