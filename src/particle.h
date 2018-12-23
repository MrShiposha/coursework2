#ifndef CG_SEM5_PARTICLE_H
#define CG_SEM5_PARTICLE_H

#include <glm/glm.hpp>

struct Particle
{
    glm::vec4 position;
    glm::vec4 color;
    float alpha;
    float size;
    float rotation;
    
    glm::vec3 velocity;
    float rotation_speed;
};

#endif // CG_SEM5_PARTICLE_H