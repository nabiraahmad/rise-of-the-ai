#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

enum EntityType { PLAYER, PLATFORM, ENEMY, DOOR };
enum AIType { WALKER, GUARD, JUMPER };
enum AIState { IDLE, WALKING, JUMPING };

class Entity {
public:
    EntityType entity_type;
    AIType ai_type;
    AIState ai_state;

    glm::mat4 model_matrix;
    glm::vec3 position;
    glm::vec3 movement;
    glm::vec3 acceleration;
    glm::vec3 velocity;

    bool jump = false;
    float jump_power = 0;
    float speed;

    GLuint textureID;

    float width = 1;
    float height = 1;
    
    int* walking_right = NULL;
    int* walking_left = NULL;

    int* animation_indices = NULL;
    int animation_frames = 0;
    int animation_index = 0;
    float animation_time = 0;
    int animation_cols = 0;
    int animation_rows = 0;

    bool is_active = true;

    bool collidedTop = false;
    bool collidedBottom = false;
    bool collidedLeft = false;
    bool collidedRight = false;

    Entity* object_collided;
    bool game_end;
    bool escaped;
    float jumping_time;

    Entity();

    bool check_collision(Entity* other);
    void check_collision_y(Entity* objects, int objectCount);
    void check_collision_x(Entity* objects, int objectCount);

    void update(float deltaTime, Entity* player, Entity* platforms, int platformCount, Entity* enemies, int enemyCount, Entity* finalFlag);
    void render(ShaderProgram* program);
    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint textureID, int index);

    void ai(Entity* player);
    void ai_walker();
    void ai_guard(Entity* player);
    void ai_jumper();
};
