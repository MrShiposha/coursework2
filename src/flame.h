#ifndef CG_SEM5_FLAME_H
#define CG_SEM5_FLAME_H

#include <vector>

#include "actor.h"
#include "particle.h"
#include "texture2d.h"

class Flame : public Actor
{
public:
    Flame
    (
        std::string_view id,
        size_t particle_count,
        double flame_radius,
        double alpha_damping,
        double size_damping,
        double alpha_threshold,
        const glm::vec3 &emitter_position,
        const glm::vec3 &min_velocity,
        const glm::vec3 &max_velocity,
        std::shared_ptr<Texture2D> texture
    );

    void update(float delta_time);

    const std::vector<Particle> &get_particles() const;

    std::shared_ptr<Texture2D> get_texture() const;

    VkDescriptorSet &get_texture_descriptor_set();

private:
    void initialize_particle(Particle &particle);

    std::vector<Particle> particles;
    double flame_radius;
    double alpha_damping;
    double size_damping;
    double alpha_threshold;
    glm::vec3 emitter_position;
    glm::vec3 min_velocity;
    glm::vec3 max_velocity;

    std::shared_ptr<Texture2D> texture;
    VkDescriptorSet texture_descriptor_set;
};

#endif // CG_SEM5_FLAME_H