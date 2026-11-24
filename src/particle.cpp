#include <particle.h>
#include <utility>

void swap_particles(Particle &a, Particle &b) {
    std::swap(a.r, b.r);
    std::swap(a.g, b.g);
    std::swap(a.b, b.b);
    std::swap(a.a, b.a);
    std::swap(a.type, b.type);
}