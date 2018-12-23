#include <glm/gtc/matrix_transform.hpp>

#include "actorcontroller.h"

ActorController::ActorController
(
    float movement_speed, 
    float rotation_speed,
    const glm::vec3 &position,
    const glm::vec3 &rotation
)
: controlled_actor(nullptr),
position(position), rotation(rotation),
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

void ActorController::rotate(float pitch, float yaw)
{
    rotation.x += pitch * rotation_speed;
    rotation.y += yaw * rotation_speed;
    rotation.z = 0;
    // controlled_actor->rotate(glm::radians(yaw * rotation_speed), glm::vec3(1.f, 0.f, 0.f));
    // controlled_actor->rotate(glm::radians(-pitch * rotation_speed), glm::vec3(0.f, 1.f, 0.f));
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

    // if(movement == Movement::NO)
        // return;

    glm::vec3 movement_direction;
    movement_direction.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
    movement_direction.y = sin(glm::radians(rotation.x));
    movement_direction.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
    movement_direction = glm::normalize(movement_direction);

    float speed = delta_time * movement_speed;

    glm::vec3 right_direction = glm::cross(movement_direction, glm::vec3(0.f, 1.f, 0.f));

    if(movement & Movement::FORWARD)
        position += movement_direction * speed;
    if(movement & Movement::BACKWARD)
        position -= movement_direction * speed;
    if(movement & Movement::LEFT)
        position -= right_direction * speed;
    if(movement & Movement::RIGHT)
        position += right_direction * speed;

    glm::mat trans_matrix = glm::translate(glm::mat4(1.f), position);
    glm::mat4 rot_matrix(1.f);
    rot_matrix = glm::rotate(rot_matrix, glm::radians(rotation.x), glm::vec3(1.f, 0.f, 0.f));
    rot_matrix = glm::rotate(rot_matrix, glm::radians(rotation.y), glm::vec3(0.f, 1.f, 0.f));
    rot_matrix = glm::rotate(rot_matrix, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));

    controlled_actor->set_model_matrix(rot_matrix * trans_matrix);
}