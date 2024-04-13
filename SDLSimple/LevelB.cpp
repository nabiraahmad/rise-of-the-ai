/**
* Author: Nabira Ahmad
* Assignment: Platformer
* Date due: 2024-04-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 30
#define LEVEL_HEIGHT 5
extern int g_lives_left;
extern ShaderProgram g_program;
extern int g_text_texture_id;

unsigned int LevelB_DATA[] =
{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 12, 12,  0,  0,  0,  0,  0,  0, 62, 62, 62, 62, 62, 63, 63, 63, 63,
    0,  0,  0,  0, 12, 12, 12, 12, 12, 12,  0,  0,  0,  0,  0,  0,  0,  0, 12, 12,  0, 64, 64, 64, 64, 64, 64, 64, 63, 63,
    2, 2, 2,  0,  0,  0,  0,  0,  0,  0,  0,  0, 63, 63, 63, 63, 64,  0,  0,  0,  0, 64, 64, 63, 63, 63, 64, 64, 64, 63,
    2, 2, 2, 2,  0, 0, 2,  2,  0,  0,  0, 64, 64, 64, 64, 64, 123, 0,  0,  0,  0, 64, 64, 64, 62, 62, 64, 64, 64, 64,
    2, 2, 2, 2,  0, 2, 2,  2,  2,  0,  0, 64, 64, 64, 63, 63, 63,  0,  0,  0,  0, 64, 64, 64, 64, 64, 64, 64, 64, 64

};

LevelB::~LevelB()
{
    delete[] m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeMusic(m_state.bgm);
}

void LevelB::initialise()
{
    m_state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("Tileset.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LevelB_DATA, map_texture_id, 1.0f, 20, 9);

 
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->set_pos(glm::vec3(0.0f, 0.0f, 0.0f));
    m_state.player->set_movement(glm::vec3(0.0f));
    m_state.player->set_speed(3.0f);
    m_state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_state.player->m_texture_id = Utility::load_texture("green.png");


    m_state.player->m_walking[m_state.player->LEFT] = new int[3]{3,4,5};
    m_state.player->m_walking[m_state.player->RIGHT] = new int[3]{6,7,8};
    m_state.player->m_animation_indices = m_state.player->m_walking[m_state.player->RIGHT];  // start walking right
    m_state.player->m_animation_frames = 3;
    m_state.player->m_animation_index = 0;
    m_state.player->m_animation_time = 0.0f;
    m_state.player->m_animation_cols = 3;
    m_state.player->m_animation_rows = 4;
    m_state.player->set_height(0.8f);
    m_state.player->set_width(0.4f);
    m_state.player->m_jump_force = 5.5f;

    
    GLuint enemy_texture_id = Utility::load_texture("32x32-bat-sprite.png");
    m_state.enemies = new Entity[ENEMY_COUNT];
    
    for (int i = 0; i < ENEMY_COUNT; ++i) {
        m_state.enemies[i].set_entity_type(ENEMY);
        m_state.enemies[i].m_texture_id = enemy_texture_id;
        m_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_state.enemies[i].set_speed(2.5f);
        m_state.enemies[i].set_acceleration(glm::vec3(0.0f, -12.0f, 0.0f));
        m_state.enemies[i].set_height(0.4f);
        m_state.enemies[i].set_width(0.6f);
        m_state.enemies[i].m_walking[m_state.enemies[i].LEFT] = new int[3]{13,14,15};
        m_state.enemies[i].m_walking[m_state.enemies[i].RIGHT] = new int[3]{5,6,7};
        m_state.enemies[i].m_animation_indices = m_state.enemies[i].m_walking[m_state.enemies[i].RIGHT];
        m_state.enemies[i].m_animation_frames = 3;
        m_state.enemies[i].m_animation_index = 0;
        m_state.enemies[i].m_animation_time = 0.0f;
        m_state.enemies[i].m_animation_cols = 4;
        m_state.enemies[i].m_animation_rows = 4;
    }
    
    // first enemy -> jumper
    m_state.enemies[0].set_ai_type(JUMPER);
    m_state.enemies[0].set_pos(glm::vec3(9.0f, -1.0f, 0.0f));
    m_state.enemies[0].m_jump_force = 0.25f;
    
    // second enemy -> guard
    m_state.enemies[1].set_ai_type(GUARD);
    m_state.enemies[1].set_ai_state(IDLE);
    m_state.enemies[1].set_pos(glm::vec3(15.0f, 0.0f, 0.0f));

    // third enemy -> jumper
    m_state.enemies[2].set_ai_type(JUMPER);
    m_state.enemies[2].set_pos(glm::vec3(25.0f, 2.0f, 0.0f));
    m_state.enemies[2].m_jump_force = 0.25f;
    
    // fourth enemy -> guard
    m_state.enemies[3].set_ai_type(GUARD);
    m_state.enemies[3].set_pos(glm::vec3(27.0f, 2.0f, 0.0f));


    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_PlayMusic(m_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 8.0f);
    m_state.jump_sfx = Mix_LoadWAV("boing.wav");
    
}

void LevelB::update(float delta_time)
{
    m_state.player->update(delta_time, m_state.player, m_state.enemies, ENEMY_COUNT, m_state.map);

    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_state.enemies[i].update(delta_time, m_state.player, m_state.player, 1, m_state.map);
    }
    if (m_state.player->m_enemies_killed == ENEMY_COUNT) m_state.next_scene_id = 3;
}

void LevelB::render(ShaderProgram* program)
{
    // for every level
    glClearColor(0.1f, 0.2f, 0.1f, 1.0f);
    std::string livesText = "Lives: " + std::to_string(g_lives_left);
    Utility::draw_text(&g_program, g_text_texture_id, livesText, 0.45f, 0.01f, glm::vec3(-4.5f, 3.5f, 0.0f));

    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_state.enemies[i].render(program);
    }

    m_state.player->m_enemies_killed = 0;
    for (int i = 0; i < ENEMY_COUNT; i++) {
        if (!m_state.enemies[i].is_active()) {
            m_state.player->m_enemies_killed += 1;
        }
        else if (m_state.enemies[i].get_pos().y < -5.0f) {
            m_state.player->m_enemies_killed ++;
        }
    }

    if (m_state.player->m_num_deaths == 3) {
        m_state.player->deactivate();
    }
    else {
        m_state.player->render(program);
    }

    if (m_state.player->get_pos().y < -5.0f) {
        m_state.player->m_num_deaths += 1;
        m_state.player->set_pos(glm::vec3(0.0f, 0.0f, 0.0f));
    }

    m_state.map->render(program);
}
