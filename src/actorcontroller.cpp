#include "actorcontroller.h"

ActorController::ActorController(float movement_speed, float rotation_speed)
: controlled_actor(nullptr), movement_direction(glm::vec3(0.f, 0.f, 1.f)),
movement_speed(movement_speed), rotation_speed(rotation_speed),
movement(Movement::NO)
{}

float ActorController::get_movement_speed() const
{
    return movement_speed;
}

float ActorController::get_rotation_speed() const
{
    return rotation_speed;
}


void ActorController::set_movement_speed(float speed)
{
    movement_speed = speed;
}

void ActorController::set_rotation_speed(float speed)
{
    rotation_speed = speed;
}


ActorController::Movement ActorController::get_movement() const
{
    return movement;
}

void ActorController::set_movement(Movement movement)
{
    this->movement = movement;
}

glm::vec3 ActorController::get_movement_direction() const
{
    return movement_direction;
}

void ActorController::rotate(float pitch, float yaw)
{
    controlled_actor->rotate(glm::radians(yaw * rotation_speed), glm::vec3(1.f, 0.f, 0.f));
    controlled_actor->rotate(glm::radians(-pitch * rotation_speed), glm::vec3(0.f, 1.f, 0.f));
}

std::shared_ptr<Actor> ActorController::get_actor() const
{
    return controlled_actor;
}

void ActorController::set_actor(std::shared_ptr<Actor> actor)
{
    controlled_actor = actor;
}

void ActorController::update(float delta_time)
{
    if(controlled_actor == nullptr)
        return;

    if(movement == Movement::NO)
        return;

    float speed = delta_time * movement_speed;

    glm::vec3 translation(0.f);
    glm::vec3 left_direction = glm::cross(movement_direction, glm::vec3(0.f, 1.f, 0.f));

    if(movement & Movement::FORWARD)
        translation += movement_direction * speed;
    if(movement & Movement::BACKWARD)
        translation -= movement_direction * speed;
    if(movement & Movement::LEFT)
        translation += left_direction * speed;
    if(movement & Movement::RIGHT)
        translation -= left_direction * speed;

    controlled_actor->translate(translation);
}