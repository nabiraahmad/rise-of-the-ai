//    Author: Nabira Ahmad
//    Assignment: Rise of the AI
//    Date due: 2024-03-30, 11:59pm
//    I pledge that I have completed this assignment without
//    collaborating with anyone else, in conformance with the
//    NYU School of Engineering Policies and Procedures on
//    Academic Misconduct.

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
#include <SDL_mixer.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"
#include <vector>

#define PLATFORM_COUNT 27
#define ENEMY_COUNT 3
#define FIXED_TIMESTEP 0.0166666f

struct GameState {
    Entity* player;
    Entity* enemies;
    Entity* platforms;
    Entity* door;
    
    bool start_game = true;
    bool no_kills = true;
    bool end_game = false;
    bool escaped = false;
};
const int   WINDOW_WIDTH = 640,
            WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
            BG_BLUE = 0.549f,
            BG_GREEN = 0.9059f,
            BG_OPACITY = 1.0f;

const int   VIEWPORT_X = 0,
            VIEWPORT_Y = 0,
            VIEWPORT_WIDTH = WINDOW_WIDTH,
            VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const int   CD_QUAL_FREQ    = 44100,
            AUDIO_CHAN_AMT  = 2,
            AUDIO_BUFF_SIZE = 4096;

float lastTicks = 0;
float accumulator = 0.0f;

// music
Mix_Music* g_music;
Mix_Chunk* g_bouncing_sfx;

// sfx
const char BGM_FILEPATH[] = "Acid Network.mp3";
const int    LOOP_FOREVER = -1;  // -1 means loop forever in Mix_PlayMusic; 0 means play once and loop zero times
const int PLAY_ONCE   =  0,
          NEXT_CHNL   = -1,  // next available channel
          MUTE_VOL    =  0,
          MILS_IN_SEC = 1000,
          ALL_SFX_CHN = -1;

GameState state;
SDL_Window* displayWindow;
bool game_is_running = true;
ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

// load texture function
GLuint load_texture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return textureID;
}

// initialize function
void Initialize() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    
    Mix_OpenAudio(
            CD_QUAL_FREQ,        // the frequency to playback audio at (in Hz)
            MIX_DEFAULT_FORMAT,  // audio format
            AUDIO_CHAN_AMT,      // number of channels (1 is mono, 2 is stereo, etc).
            AUDIO_BUFF_SIZE      // audio buffer size in sample FRAMES (total samples divided by channel count)
            );
    
    g_bouncing_sfx = Mix_LoadWAV("jump.wav");

        Mix_PlayChannel(
            NEXT_CHNL,       // using the first channel that is not currently in use...
            g_bouncing_sfx,  // ...play this chunk of audio...
            PLAY_ONCE        // ...once.
            );

        // Fade in (from 0 to full volume) over 1 second
        Mix_FadeInChannel(
            NEXT_CHNL,       // using the first channel that is not currently in use...
            g_bouncing_sfx,  // ...fade in this chunk of audio from volume 0 to max volume...
            PLAY_ONCE,       // ...once...
            MILS_IN_SEC      // ...over 1000 miliseconds.
            );

        Mix_Volume(
            ALL_SFX_CHN,        // Set all channels...
            MIX_MAX_VOLUME / 2  // ...to half volume.
            );

        Mix_VolumeChunk(
            g_bouncing_sfx,     // Set the volume of the bounce sound...
            MIX_MAX_VOLUME / 4  // ... to 1/4th.
            );

    displayWindow = SDL_CreateWindow("ESCAPE THE DUNGEON", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    program.load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.set_projection_matrix(projectionMatrix);
    program.set_view_matrix(viewMatrix);

    glUseProgram(program.get_program_id());

    // dark purplish
    glClearColor(0.1f, 0.0f, 0.1f, 1.0f);

    // music stuff
    g_music = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(g_music, LOOP_FOREVER);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
        
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // player stuff
    state.player = new Entity();
    state.player->entity_type = PLAYER;
    state.player->acceleration = glm::vec3(0, -9.81f, 0);
    state.player->speed = 1.5f;
    state.player->position = glm::vec3(-4.5, 2, 0);
    state.player->movement = glm::vec3(0);
    state.player->textureID = load_texture("walk and idle.png");

    // animation left and right
    state.player->walking_right = new int[8]{17, 18, 19, 20, 21, 22, 23, 24};
    state.player->walking_left = new int[8]{9, 10, 11, 12, 13, 14, 15, 16};

    // start walking right
    state.player->animation_indices = state.player->walking_right;
    state.player->animation_frames = 7;
    state.player->animation_index = 0;
    state.player->animation_time = 0;
    state.player->animation_cols = 8;
    state.player->animation_rows = 3;
    
    // collision dimensions & jumping force
    state.player->jump_power = 5.0f;
    state.player->height = 0.8f;
    state.player->width = 0.4f;

    // platform stuff
    state.platforms = new Entity[PLATFORM_COUNT];

    GLuint platform_texture_id = load_texture("darkcastle.png");
    
    // build all the platforms (3 ish levels)
    for (int i = 0; i < PLATFORM_COUNT / 4; i++) {
        state.platforms[i].position = glm::vec3(-4+i, 1, 0);
    }
    int platform_num = 0;
    for (int i = PLATFORM_COUNT / 4; i < PLATFORM_COUNT / 2; i++) { // Corrected range
        state.platforms[i].position = glm::vec3(4 + platform_num, -1, 0);
        platform_num++;
    }

    platform_num = 0;
    for (int i = PLATFORM_COUNT / 2; i < (3 * PLATFORM_COUNT) / 4; i++) { // Corrected range
        state.platforms[i].position = glm::vec3(-1 + platform_num, -2, 0);
        platform_num++;
    }

    platform_num = 0;
    for (int i = (3 * PLATFORM_COUNT) / 4; i < PLATFORM_COUNT; i++) {
        state.platforms[i].position = glm::vec3(-5 + platform_num, -4, 0);
        platform_num++;
    }

    // platform the player lands on initially
    state.platforms[PLATFORM_COUNT - 1].position = glm::vec3(-4.5, 2, 0);

    // updating the platforms and initializing them as the PLATFORM type
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].entity_type = PLATFORM;
        state.platforms[i].textureID = platform_texture_id;
        state.platforms[i].update(0, NULL, NULL, 0, NULL, 0, NULL);
    }

    // escape door stuff
    state.door = new Entity();
    GLuint door_texture_id = load_texture("door.png");
    state.door->entity_type = DOOR;
    state.door->textureID = door_texture_id;
    
    state.door->width = 0.5;
    state.door->position = glm::vec3(-4.5, -3, 0);
    state.door->update(0, NULL, NULL, 0, NULL, 0, NULL);


    // enemy stuff
    state.enemies = new Entity[ENEMY_COUNT];
    GLuint enemy_texture_id = load_texture("32x32-bat-sprite.png");

    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].entity_type = ENEMY;
        state.enemies[i].textureID = enemy_texture_id;
        
        state.enemies[i].speed = 1.0;
        state.enemies[i].width = 0.6;
        state.enemies[i].height = 0.4f;

        state.enemies[i].walking_right = new int[3]{5,6,7};
        state.enemies[i].walking_left = new int[3]{13,14,15};

        state.enemies[i].animation_frames = 3;
        state.enemies[i].animation_index = 0;
        state.enemies[i].animation_time = 0;
        state.enemies[i].animation_cols = 4;
        state.enemies[i].animation_rows = 4;
        state.enemies[i].acceleration = glm::vec3(0, -9.81f, 0);
        state.enemies[i].animation_indices = state.enemies->walking_right;
    }
    
    // first enemy is a guard
    state.enemies[0].position = glm::vec3(-.75, 2.75, 0);
    state.enemies[0].ai_type = GUARD;
    state.enemies[0].ai_state = IDLE;

    // second enemy is a walker
    state.enemies[1].animation_indices = state.enemies->walking_left;
    state.enemies[1].position = glm::vec3(0.75, 0, 0);
    state.enemies[1].movement = glm::vec3(-0.75, 0, 0);
    state.enemies[1].ai_type = WALKER;
    state.enemies[1].ai_state = WALKING;

    // third enemy is a jumper
    state.enemies[2].position = glm::vec3(-3.75, -3, 0);
    state.enemies[2].movement = glm::vec3(0.75, 0, 0);
    state.enemies[2].jump_power = 3;
    state.enemies[2].ai_type = JUMPER;
    state.enemies[2].ai_state = JUMPING;


}

// draw text function
void draw_text(ShaderProgram* program, GLuint fontTextureID, std::string text,
    float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;

    std::vector<float> vertices;
    std::vector<float> texCoords;

    for (int i = 0; i < text.size(); i++) {

        int index = (int)text[i];
        float offset = (size + spacing) * i;
        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;
        vertices.insert(vertices.end(), {
         offset + (-0.5f * size), 0.5f * size,
         offset + (-0.5f * size), -0.5f * size,
         offset + (0.5f * size), 0.5f * size,
         offset + (0.5f * size), -0.5f * size,
         offset + (0.5f * size), 0.5f * size,
         offset + (-0.5f * size), -0.5f * size,
            });
        texCoords.insert(texCoords.end(), {
        u, v,
        u, v + height,
        u + width, v,
        u + width, v + height,
        u + width, v,
        u, v + height,
            });

    }
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->set_model_matrix(modelMatrix);

    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void process_input() {

    state.player->movement = glm::vec3(0);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            game_is_running = false;
            break;

        case SDL_KEYDOWN:
            state.start_game = false;
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                break;

            case SDLK_RIGHT:
                break;
                    
            case SDLK_m:
                // mute volume
                Mix_HaltMusic();
                break;

            case SDLK_SPACE:
                if (state.player->collidedBottom) {
                    // jumping sound effect
                    state.player->jump = true;
                    Mix_PlayChannel(
                                            NEXT_CHNL,       // using the first channel that is not currently in use...
                                            g_bouncing_sfx,  // ...play this chunk of audio...
                                            PLAY_ONCE        // ...once.
                                        );
                }
                break;
            }
            break;
        }
        
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    // keyboard controls
    if (keys[SDL_SCANCODE_LEFT]) {
        state.player->movement.x = -1.0f;
        state.player->animation_indices = state.player->walking_left;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        state.player->movement.x = 1.0f;
        state.player->animation_indices = state.player->walking_right;
    }


    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }

}


void update() {

    if (state.end_game) {
        return;
    }

    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
       
        for (int i = 0; i < ENEMY_COUNT; i++) {
            state.enemies[i].update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT, state.door);
        }
        
        state.player->update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT, state.door);

        deltaTime -= FIXED_TIMESTEP;
    }


    accumulator = deltaTime;
    state.end_game = state.player->game_end;
    state.escaped = state.player->escaped;

    // if every enemy is active, player has 0 kills
    for (int i = 0; i < ENEMY_COUNT; i++) {
        if (state.enemies[i].is_active == false)
            state.no_kills = false;
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // render platforms, enemies, player, door
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].render(&program);
    }

    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].render(&program);
    }
    
    state.door->render(&program);
    state.player->render(&program);

    GLuint font_texture_id = load_texture("font1.png");
    
    // if the game just started or there are no kills and the game is NOT over, show game rules
    if (state.start_game or state.no_kills and not state.end_game) {
        draw_text(&program, font_texture_id, "ESCAPE THE DUNGEON", 0.5, -0.20f, glm::vec3(-2, 0, 0));
        draw_text(&program, font_texture_id, "KILL ALL ENEMIES", 0.5, -0.20f, glm::vec3(-1.6, -0.5, 0));
    }

    if (state.end_game)
    {
        if (state.escaped) {
            // count how many enemies were killed
            bool killed_all_enemies = true;
            for (int i = 0; i < ENEMY_COUNT; i++) {
                if (state.enemies[i].is_active)
                    killed_all_enemies = false;
            }
            
            // if the game is over and the player escaped and killed all enemies, show win message
            if (killed_all_enemies)
                draw_text(&program, font_texture_id, "YOU MADE IT!", 0.6, -0.20f, glm::vec3(-1.5, -0.3, 0));
            else
            // if the game is over and the player escaped and DIDN'T kill all enemies, show loser message
                draw_text(&program, font_texture_id, "ITS LOCKED... YOU DIED!", 0.5, -0.23f, glm::vec3(-2.5,0, 0));
        }

        else
            // if the game is over and the player didn't escape, show loser message
            draw_text(&program, font_texture_id, "YOU DIED...", 0.6, -0.20f, glm::vec3(-1.2, -0.3, 0));
    }
    SDL_GL_SwapWindow(displayWindow);
}


void shutdown() {
    // free music
    Mix_FreeChunk(g_bouncing_sfx);
        Mix_FreeMusic(g_music);
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (game_is_running) {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
