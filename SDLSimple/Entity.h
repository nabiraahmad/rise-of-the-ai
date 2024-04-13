#pragma once
#include "Map.h"

// enemy ai types
enum EntityType { PLAYER, ENEMY };
enum AIType { WALKER, GUARD, JUMPER };
enum AIState { WALKING, IDLE };

class Entity
{
private:
    bool m_is_active = true;

    int* m_animation_right = NULL,
    * m_animation_left = NULL;

    // movement
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;
    float     m_speed;
    glm::vec3 m_movement;
    glm::mat4 m_model_matrix;

    // enemy ai
    EntityType m_entity_type;
    AIType     m_ai_type;
    AIState    m_ai_state;

    float m_width = 1;
    float m_height = 1;

public:
    // player stuff and animation
    int m_enemies_killed = 0;
    int m_num_deaths = 0;
    static const int    SECONDS_PER_FRAME = 4;
    static const int    LEFT = 0,
                        RIGHT = 1;
    
    int** m_walking = new int* [2]
        {
            m_animation_left,
                m_animation_right,
        };

    int m_animation_frames = 0,
        m_animation_index = 0,
        m_animation_cols = 0,
        m_animation_rows = 0;

    int* m_animation_indices = NULL;
    float   m_animation_time = 0.0f;

    bool  m_is_jumping = false;
    float m_jump_force = 0;

    bool m_collided_top = false;
    bool m_collided_bottom = false;
    bool m_collided_left = false;
    bool m_collided_right = false;
    GLuint    m_texture_id;

    // methods
    Entity();
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    void update(float delta_time, Entity* player, Entity* objects, int object_count, Map* map);
    void render(ShaderProgram* program);

    bool const check_collision(Entity* other) const;
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);

    void const check_collision_y(Map* map);
    void const check_collision_x(Map* map);

    void move_left() { m_movement.x = -1.0f; };
    void move_right() { m_movement.x = 1.0f; };

    void ai_activate(Entity* player);
    void ai_walk();
    void ai_guard(Entity* player);
    void ai_jump();

    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false; };

    // getters
    EntityType const get_entity_type()    const { return m_entity_type; };
    AIType     const get_ai_type()        const { return m_ai_type; };
    AIState    const get_ai_state()       const { return m_ai_state; };
    glm::vec3  const get_pos()       const { return m_position; };
    glm::vec3  const get_movement()       const { return m_movement; };
    glm::vec3  const get_velocity()       const { return m_velocity; };
    glm::vec3  const get_acc()   const { return m_acceleration; };
    float      const get_jump_force()  const { return m_jump_force; };
    float      const get_speed()          const { return m_speed; };
    int        const get_width()          const { return m_width; };
    int        const get_height()         const { return m_height; };
    bool       const is_active()      const { return m_is_active; };
    bool m_counted;

    // setters
    void const set_entity_type(EntityType new_entity_type) { m_entity_type = new_entity_type; };
    void const set_ai_type(AIType new_ai_type) { m_ai_type = new_ai_type; };
    void const set_ai_state(AIState new_state) { m_ai_state = new_state; };
    void const set_pos(glm::vec3 new_position) { m_position = new_position; };
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; };
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; };
    void const set_speed(float new_speed) { m_speed = new_speed; };
    void const set_jumping_power(float new_jumping_power) { m_jump_force = new_jumping_power; };
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; };
    void const set_width(float new_width) { m_width = new_width; };
    void const set_height(float new_height) { m_height = new_height; };
};
