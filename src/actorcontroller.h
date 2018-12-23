#ifndef CG_SEM5_ACTORCONTROLLER_H
#define CG_SEM5_ACTORCONTROLLER_H

#include <memory>
#include <glm/glm.hpp>

#include "actor.h"

class ActorController
{
public:
    enum Movement
    {
        NO       = 0x0,
        FORWARD  = 0x1,
        BACKWARD = 0x2,
        LEFT     = 0x4,
        RIGHT    = 0x8
    };

    ActorController
    (
        float movement_speed, 
        float rotation_speed,
        const glm::vec3 &position,
        const glm::vec3 &rotation
    );

    float get_movement_speed() const;
    float get_rotation_speed() const;

    void set_movement_speed(float);
    void set_rotation_speed(float);

    Movement get_movement() const;
    void set_movement(Movement);

    void rotate(float pitch, float yaw);

    std::shared_ptr<Actor> get_actor() const;
    void set_actor(std::shared_ptr<Actor>);

    void update(float delta_time);

private:
    std::shared_ptr<Actor> controlled_actor;
    glm::vec3 position;
    glm::vec3 rotation;

    float movement_speed;
    float rotation_speed;

    Movement movement;
};

#endif // CG_SEM5_ACTORCONTROLLER_H