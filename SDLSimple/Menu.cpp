/**
* Author: Nabira Ahmad
* Assignment: Platformer
* Date due: 2024-04-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Menu.h"
#include "Utility.h"

#define LEVEL_WIDTH 30
#define LEVEL_HEIGHT 5

unsigned int Menu_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


Menu::~Menu()
{
    delete[] m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeMusic(m_state.bgm);
}

void Menu::initialise()
{
    m_state.next_scene_id = -1;

    GLuint map_texture_id = Utility::load_texture("Tileset.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, Menu_DATA, map_texture_id, 1.0f, 20, 9);

    // player stuff
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->set_pos(glm::vec3(0.0f, 0.0f, 0.0f));
    m_state.player->set_movement(glm::vec3(0.0f));
    m_state.player->set_speed(3.0f);
    m_state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_state.player->m_texture_id = Utility::load_texture("green.png");

    // Walking
    m_state.player->m_walking[m_state.player->LEFT] = new int[3]{3,4,5};
    m_state.player->m_walking[m_state.player->RIGHT] = new int[3]{6,7,8};

    // start walking right
    m_state.player->m_animation_indices = m_state.player->m_walking[m_state.player->RIGHT];
    m_state.player->m_animation_frames = 3;
    m_state.player->m_animation_index = 0;
    m_state.player->m_animation_time = 0.0f;
    m_state.player->m_animation_cols = 3;
    m_state.player->m_animation_rows = 4;
    m_state.player->set_height(0.8f);
    m_state.player->set_width(0.4f);
    m_state.player->m_jump_force = 5.4f;

    // enemy stuff
    GLuint enemy_texture_id = Utility::load_texture("32x32-bat-sprite.png");

    m_state.enemies = new Entity[ENEMY_COUNT];
    
    for (int i = 0; i < ENEMY_COUNT; ++i) {
        m_state.enemies[i].set_entity_type(ENEMY);
        m_state.enemies[i].m_texture_id = enemy_texture_id;
        m_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_state.enemies[i].set_speed(1.5f);
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

    // music
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    m_state.bgm = Mix_LoadMUS("lofi.mp3");
    Mix_PlayMusic(m_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 8.0f);

    m_state.jump_sfx = Mix_LoadWAV("boing.wav");
}

void Menu::update(float delta_time)
{}

void Menu::render(ShaderProgram* program)
{
    // black menu
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_state.map->render(program);

}
