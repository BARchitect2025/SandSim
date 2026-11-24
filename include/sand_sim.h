#ifndef SAND_PHYSICS_H_
#define SAND_PHYSICS_H_

#include <vector>
#include <random>

#include "particle.h"

class SandSim {
public:
    void init(int width, int height, std::mt19937 &rand_gen);
    void update(std::mt19937 &rand_gen);

    void add_particles(ParticleType type, char r, char g, char b, char a, int x, int y, int radius);

    const std::vector<Particle>& get_particles() const;

    int get_max_particles();
private:
    int width;
    int height;

    bool left_to_right;

    std::vector<bool> updated;
    std::vector<Particle> particles;

    MoveFlags can_move(int i, int j);
};

#endif