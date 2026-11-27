#ifndef PARTICLE_H_
#define PARTICLE_H_

#include <cstdint>

enum class ParticleType : uint8_t {
    SAND = 0,
    WATER = 1,
    CLONER = 2,
    EMPTY = 3
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

struct Cloner : Particle {
    ParticleType cloning;
};

void swap_particles(Particle &a, Particle &b);

#endif