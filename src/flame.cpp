#include <cmath>
#include <random>

#include "flame.h"

float rnd(float range)
{
    static std::default_random_engine rnd_engine;
    std::uniform_real_distribution<float> rnd_dist(0.0f, range);
	return rnd_dist(rnd_engine);
}

Flame::Flame
(
    size_t particle_count,
    double flame_radius,
    double alpha_damping,
    double size_damping,
    double alpha_threshold,
    const glm::vec3 &emitter_position,
    const glm::vec3 &min_velocity,
    const glm::vec3 &max_velocity,
    std::shared_ptr<Texture2D> texture
) : particles(particle_count), flame_radius(flame_radius), 
alpha_damping(alpha_damping), 
size_damping(size_damping),
alpha_threshold(alpha_threshold),
emitter_position(emitter_position),
min_velocity(min_velocity),
max_velocity(max_velocity),
texture(texture)
{
    for (auto &&particle : particles)
    {
        initialize_particle(particle);
        particle.alpha = 1.f - (std::abs(particle.position.y) / flame_radius * 2.f);
    }
}

void Flame::update(float delta_time)
{
    for(auto &&particle : particles)
    {
        particle.position.y -= particle.velocity.y * delta_time * 3.5f;
        particle.alpha      += delta_time * alpha_damping;
        particle.size       -= delta_time * size_damping;

        particle.rotation += delta_time * particle.rotation_speed;

        if(particle.alpha > alpha_threshold)
            initialize_particle(particle);
    }

    this->mark_changed();
}

const std::vector<Particle> &Flame::get_particles() const
{
    return particles;
}

void Flame::initialize_particle(Particle &particle)
{
    particle.velocity       = glm::vec4(0.0f, min_velocity.y + rnd(max_velocity.y - min_velocity.y), 0.0f, 0.0f);
    particle.alpha          = rnd(0.75f);
    particle.size           = 1.0f + rnd(0.5f);
    particle.color          = glm::vec4(1.0f);
    particle.rotation       = rnd(2.0f * float(M_PI));
    particle.rotation_speed = rnd(2.0f) - rnd(2.0f);

    float theta = rnd(2.0f * float(M_PI));
    float phi = rnd(float(M_PI)) - float(M_PI) / 2.0f;
    float r = rnd(flame_radius);

    particle.position.x = r * cos(theta) * cos(phi);
    particle.position.y = r * sin(phi);
    particle.position.z = r * sin(theta) * cos(phi);

    particle.position += glm::vec4(emitter_position, 0.0f);
}