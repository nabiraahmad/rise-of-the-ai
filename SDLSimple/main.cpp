/**
* Author: Nabira Ahmad
* Assignment: Platformer
* Date due: 2024-04-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "Menu.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"

// ––––– CONSTANTS ––––– //
const int WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
            BG_BLUE = 0.549f,
            BG_GREEN = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const char FONT_SPRITE_FILEPATH[] = "font1.png";
const int FONTBANK_SIZE = 16;

const float MILLISECONDS_IN_SECOND = 1000.0;

// ––––– GLOBAL VARIABLES ––––– //
Scene* g_current_scene;
Menu* g_menu;
LevelA* g_levelA;
LevelB* g_levelB;
LevelC* g_levelC;

Scene* g_levels[4];

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

GLuint g_text_texture_id;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

bool g_is_colliding_bottom = false;

int g_num_lives = 3;
int g_lives_left = 3;
int g_game_death_num = 0;
bool g_game_over = false;


void switch_to_scene(Scene* scene)
{
    g_current_scene = scene;
    g_current_scene->initialise();
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("PLATFORMER",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(g_program.get_program_id());
    g_text_texture_id = Utility::load_texture(FONT_SPRITE_FILEPATH);

    g_menu = new Menu();
    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();

    g_levels[0] = g_menu;
    g_levels[1] = g_levelA;
    g_levels[2] = g_levelB;
    g_levels[3] = g_levelC;
    
    // start with the menu screen
    switch_to_scene(g_levels[0]);
}

void process_input()
{
    g_current_scene->m_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                g_game_is_running = false;
                break;

            case SDLK_SPACE:
                // jumping
                if (g_game_over) {
                        return;
                    }
                if (g_current_scene->m_state.player->m_collided_bottom)
                {
                    // play sound effect
                    g_current_scene->m_state.player->m_is_jumping = true;
                    Mix_PlayChannel(-1, g_current_scene->m_state.jump_sfx, 0);
                }
                break;

            case SDLK_RETURN:
                // first level is A, and in level A we switch to other levels
                if (g_current_scene == g_menu) {
                    g_current_scene->m_state.next_scene_id = 1;
                    switch_to_scene(g_levelA);
                }
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        // don't want player to move if game is over
        if (g_game_over) {
                return;
            }
        g_current_scene->m_state.player->move_left();
        g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        // don't want player to move if game is over
        if (g_game_over) {
                return;
            }
        g_current_scene->m_state.player->move_right();
        g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->RIGHT];
    }

    if (glm::length(g_current_scene->m_state.player->get_movement()) > 1.0f)
    {
        g_current_scene->m_state.player->set_movement(glm::normalize(g_current_scene->m_state.player->get_movement()));
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);

        g_is_colliding_bottom = g_current_scene->m_state.player->m_collided_bottom;

        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;
    g_view_matrix = glm::mat4(1.0f);
    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->m_state.player->get_pos().x, 0.0f, 0.0f));

    // keep track of the num of deaths from each level
    if (g_game_death_num > g_current_scene->m_state.player->m_num_deaths && g_current_scene->m_state.player->m_num_deaths == 0) {
        g_current_scene->m_state.player->m_num_deaths = g_game_death_num;
    }

    if (g_game_death_num != g_current_scene->m_state.player->m_num_deaths) {
        g_game_death_num = g_current_scene->m_state.player->m_num_deaths;
    }
    // keep track of lives left
    g_lives_left = g_num_lives - g_game_death_num;

    // switch to other levels
    if (g_current_scene == g_levelA && g_current_scene->m_state.player->m_enemies_killed == g_current_scene->m_num_of_enemies)
    {
        switch_to_scene(g_levelB);
    }
    else if (g_current_scene == g_levelB && g_current_scene->m_state.player->m_enemies_killed == g_current_scene->m_num_of_enemies)
    {
        switch_to_scene(g_levelC);
    }
}

void render()
{
    g_program.set_view_matrix(g_view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(g_program.get_program_id());
    g_current_scene->render(&g_program);
    std::string livesText = "Lives: " + std::to_string(g_lives_left);
    Utility::draw_text(&g_program, g_text_texture_id, livesText, 0.45f, 0.01f, glm::vec3(-4.5f, 3.5f, 0.0f));

    // menu screen text
    if (g_current_scene == g_menu) {
        Utility::draw_text(&g_program, g_text_texture_id, std::string("PLATFORMER"), 0.5f, 0.0f, glm::vec3(-2.5f, 1.0f, 0.0f));
        Utility::draw_text(&g_program, g_text_texture_id, std::string("PRESS ENTER TO BEGIN"), 0.45f, 0.01f, glm::vec3(-4.5f, 0.0f, 0.0f));
    }
 
    // player WINS
    if (g_current_scene == g_levelC && g_current_scene->m_state.player->m_enemies_killed == g_current_scene->m_num_of_enemies)
    {
        Utility::draw_text(&g_program, g_text_texture_id, std::string("YOU MADE IT!"), 0.5f, 0.01f, glm::vec3(g_current_scene->m_state.player->get_pos().x - 2.0f, 1.0f, 0.0f));
        g_game_over = true;
    
    }
    // player LOSES
    else if (g_lives_left <= 0) {
        Utility::draw_text(&g_program, g_text_texture_id, std::string("YOU DIED!"), 0.5f, 0.01f, glm::vec3(-2.0f, 1.0f, 0.0f));
        g_game_over = true;
    }

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete g_menu;
    delete g_levelA;
    delete g_levelB;
    delete g_levelC;
}

int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();

        if (g_current_scene->m_state.next_scene_id >= 0) switch_to_scene(g_levels[g_current_scene->m_state.next_scene_id]);

        render();
    }

    shutdown();
    return 0;
}
