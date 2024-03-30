//    Author: Nabira Ahmad
//    Assignment: Rise of the AI
//    Date due: 2024-03-30, 11:59pm
//    I pledge that I have completed this assignment without
//    collaborating with anyone else, in conformance with the
//    NYU School of Engineering Policies and Procedures on
//    Academic Misconduct.

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

// entity constructor
Entity::Entity()
{
    model_matrix = glm::mat4(1.0f);
    jumping_time = 0.0f;
    object_collided = NULL;
    game_end = false;
    escaped = false;

    position = glm::vec3(0);
    movement = glm::vec3(0);;
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    speed = 0;
}

bool Entity::check_collision(Entity* other) {

    if (other == this) return false; // you shouldn't be able to collide with yourself

    if (is_active == false || other->is_active == false) return false;

    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    if (xdist < 0 && ydist < 0) {
        return true;
    }

    return false;
}

void Entity::check_collision_y(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (check_collision(object))
        {
            if (object->entity_type == ENEMY) {
                if (velocity.y > 0) {
                    velocity.y = 0;
                    collidedTop = true;
                }
                else if (velocity.y < 0) {
                    // inactivate the enemies
                    velocity.y = 0;
                    collidedBottom = true;
                    object->is_active = false;
                }
            }
            else {
                float ydist = fabs(position.y - object->position.y);
                float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
                if (velocity.y > 0) {
                    position.y -= penetrationY;
                    velocity.y = 0;
                    collidedTop = true;
                }
                else if (velocity.y < 0) {
                    position.y += penetrationY;
                    velocity.y = 0;
                    collidedBottom = true;
                }
            }
            object_collided = object;
        }
    }
}

void Entity::check_collision_x(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];

        if (check_collision(object))
        {
            if (object->entity_type == ENEMY) {
                if (velocity.x >= 0) {
                    velocity.x = 0;
                    collidedRight = true;
                }
                else if (velocity.x <= 0) {
                    velocity.x = 0;
                    collidedLeft = true;
                }
            }
            else {
                float xdist = fabs(position.x - object->position.x);
                float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
                if (velocity.x > 0) {
                    position.x -= penetrationX;
                    velocity.x = 0;
                    collidedRight = true;
                }
                else if (velocity.x < 0) {
                    position.x += penetrationX;
                    velocity.x = 0;
                    collidedLeft = true;
                }
            }
            object_collided = object;

        }
    }
}

// ai state methods

// walker method
void Entity::ai_walker() {
    
    // bounding for the walker based on its platform
    if (position.x <= -0.5) {
        // walk right
        movement = glm::vec3(.75, 0, 0);
        animation_indices = walking_right;
    }
    else if (position.x >= 3) {
        // walk left
        movement = glm::vec3(-.75, 0, 0);
        animation_indices = walking_left;
    }
}

// guard method
void Entity::ai_guard(Entity* player) {

    switch (ai_state) {
    case IDLE:
        // follow the player's movement
        if (glm::distance(position, player->position) < 3.0f) {
            ai_state = WALKING;
        }
        break;

    case WALKING:
        // move left
        if (player->position.x < position.x) {
            movement = glm::vec3(-.75, 0, 0);
            animation_indices = walking_left;
        }
        else {
            // move right
            movement = glm::vec3(.75, 0, 0);
            animation_indices = walking_right;
        }
        break;
    }
}

// jumper method
void Entity::ai_jumper() {

    // jump
    if (jumping_time >= 1.5f)
        jump = true;

    // regular walker
    if (position.x <= -3.75) {
        movement = glm::vec3(.75, 0, 0);
        animation_indices = walking_right;
    }

    else if (position.x >= 0) {
        movement = glm::vec3(-.75, 0, 0);
        animation_indices = walking_left;
    }

}

// general ai state method
void Entity::ai(Entity* player) {
    switch (ai_type) {
    case WALKER:
        ai_walker();
        break;

    case JUMPER:
        ai_jumper();
        break;
            
    case GUARD:
        ai_guard(player);
        break;
    }
}

void Entity::update(float deltaTime, Entity* player, Entity* platforms, int platformCount, Entity* enemies, int enemy_count, Entity* door)
{
    if (is_active == false) return;

    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;

    if (entity_type == ENEMY) {
        jumping_time += deltaTime;
        ai(player);
    }

    if (animation_indices != NULL) {
        if (glm::length(movement) != 0) {
            animation_time += deltaTime;

            if (animation_time >= 0.25f)
            {
                animation_time = 0.0f;
                animation_index++;
                if (animation_index >= animation_frames)
                {
                    animation_index = 0;
                }
            }
        }
        else {
            animation_index = 0;
        }
    }
    
    // jumping stuff
    if (jump) {
        jump = false;
        jumping_time = 0;
        velocity.y += jump_power;
    }

    // velocity, speed, position stuff
    velocity.x = movement.x * speed;
    velocity += acceleration * deltaTime;

    position.y += velocity.y * deltaTime; 
    check_collision_y(platforms, platformCount);

    position.x += velocity.x * deltaTime;
    check_collision_x(platforms, platformCount);

    if (entity_type == PLAYER) {
        
        // if the player collides with the door the game is over
        if (check_collision(door))
        {
            game_end = true;
            escaped = true;
            return;
        }
        
        // if the player falls off the cliff she dies!
        if (position.y < -5.0f) {
               game_end = true;
               escaped = false;
               return;
           }

        check_collision_y(enemies, enemy_count);
        check_collision_x(enemies, enemy_count);

        // if the player collided with an enemy she died!
        if (object_collided != NULL && object_collided->entity_type == ENEMY) {
            if (collidedTop || collidedRight || collidedLeft) {
                game_end = true;
                escaped = false;
                return;
            }
        }
    }
    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint textureID, int index)
{
    float u = (float)(index % animation_cols) / (float)animation_cols;
    float v = (float)(index / animation_cols) / (float)animation_rows;

    float width = 1.0f / (float)animation_cols;
    float height = 1.0f / (float)animation_rows;

    float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v };

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

    glBindTexture(GL_TEXTURE_2D, textureID);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Entity::render(ShaderProgram* program) {

    if (is_active == false) return;

    program->set_model_matrix(model_matrix);
    
    if (animation_indices != NULL) {
        draw_sprite_from_texture_atlas(program, textureID, animation_indices[animation_index]);
        return;
    }
        float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->get_position_attribute());
        
        glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDisableVertexAttribArray(program->get_position_attribute());
        glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
    }

