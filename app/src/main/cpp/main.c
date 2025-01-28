#include <stdio.h>
#include <android/log.h>

#include "raymob.h"

#define GAME_DEBUG true
#define CACTUS_COUNT 10

typedef enum GameStatus
{
    START,
    PLAYING,
    END
} GameStatus;

typedef struct main
{
    Vector2 pos;
    Vector2 size;
    Texture texture;
    bool is_dead;
} Cactus;

typedef struct
{
    int width;
    int height;
    int y;
    int ground_pos;
    int factor;
    double last_draw_time;
    double last_score_time;
    Cactus cactus[CACTUS_COUNT];
    int speed;
    int cactus_num;
    GameStatus status;
    int score;
    int high_score;
} GameState;

Texture cactus_textures[4];

typedef struct
{
    Texture texture;
    Rectangle frame_rec;
    int current_frame;
    int frames_counter;
    int frames_speed;
    int frames_count;
} StartAnimation;

typedef struct
{
    Vector2 pos;
    Vector2 init_pos;
    Vector2 size;
    int yd;
    int speed;

    Texture texture;
    Rectangle frame_rec;
    int current_frame;
    int frames_counter;
    int frames_speed;
    int frames_count;
} Player;

void render_map(GameState *state)
{
#if GAME_DEBUG
    DrawLineEx((Vector2){.x = 0, .y = state->y}, (Vector2){.y = state->y, .x = state->width}, 3, RED);
    DrawLineEx((Vector2){.x = 0, .y = state->ground_pos + state->factor}, (Vector2){.y = state->ground_pos + state->factor, .x = state->width}, 3, RED);
#endif
    DrawLineEx((Vector2){.x = 0, .y = state->ground_pos}, (Vector2){.y = state->ground_pos, .x = state->width}, 3, BLACK);
}

void render_player(GameState *state, Player *player)
{
#if GAME_DEBUG
    DrawRectangleV((Vector2){.x = player->pos.x + 25, .y = player->pos.y}, (Vector2){.x = player->size.x - 25, .y = player->size.y}, RED);
#endif
    DrawTexturePro(
        player->texture,
        player->frame_rec,
        (Rectangle){.x = player->pos.x, .y = player->pos.y, .height = player->size.y, .width = player->size.x},
        (Vector2){0, 0}, 0,
        WHITE);
}

bool is_player_jump(Player *player)
{
    return player->yd != 0;
}

void update_player(Player *player)
{
    if (GetGestureDetected() == GESTURE_TAP)
    {
        if (!is_player_jump(player))
        {
            DrawRectangle(0, 0, 100, 100, GREEN);
            player->yd = -1 * player->speed;
        }
    }
    player->pos.y += player->yd;
    if (player->init_pos.y - player->pos.y >= 350)
    {
        player->yd = 1 * player->speed;
    }
    if (player->init_pos.y - player->pos.y <= 0)
    {
        player->pos.y = player->init_pos.y;
        player->yd = 0;
    }

    // Animation

    player->frames_counter++;
    if (player->frames_counter >= (60 / player->frames_speed))
    {
        player->frames_counter = 0;
        player->current_frame++;
        if (player->current_frame > player->frames_count - 1)
            player->current_frame = 0;
        player->frame_rec.x = (float)player->current_frame * (float)player->texture.width / player->frames_count;
    }
}

void update_cactus(GameState *state)
{
    for (int i = 0; i < CACTUS_COUNT; i++)
    {
        if (!state->cactus[i].is_dead)
        {
            // __android_log_print(ANDROID_LOG_INFO, "INFO", "%f", state->cactus[i].x);
            state->cactus[i].pos.x += -1 * state->speed;
            if (state->cactus[i].pos.x + state->cactus[i].size.x < 0)
            {
                state->cactus[i].is_dead = true;
            }
        }
    }
}

Cactus create_cactus(GameState *state)
{

    Texture texture = cactus_textures[GetRandomValue(0, 3)];
    int randomScale = 2;
    Vector2 size = {
        .x = texture.width * randomScale,
        .y = texture.height * randomScale,
    };
    Cactus c = {
        .pos = (Vector2){.x = state->width + size.x + 50, .y = state->ground_pos - size.y},
        .size = size,
        .texture = texture,
    };
    return c;
}

void render_cactus(GameState *state)
{

    for (int i = 0; i < CACTUS_COUNT; i++)
    {
        if (!state->cactus[i].is_dead)
        {
            DrawTexturePro(
                state->cactus[i].texture,
                (Rectangle){.x = 0, .y = 0, .height = state->cactus[i].texture.height, .width = state->cactus[i].texture.width},
                (Rectangle){.x = state->cactus[i].pos.x, .y = state->cactus[i].pos.y, .height = state->cactus[i].size.y, .width = state->cactus[i].size.x},
                (Vector2){0, 0}, 0,
                WHITE);
            // __android_log_print(ANDROID_LOG_INFO, "INFO", "%d", state->cactus[i].x);
            // DrawRectangleV((Vector2){.x = state->cactus[i].x, .y = state->ground_pos - 100}, (Vector2){20, 100}, GREEN);
        }
    }
}

bool isColliding(Rectangle el1, Rectangle el2)
{
    return (el1.x < el2.x + el2.width && el1.x + el1.width > el2.x && el1.y < el2.y + el2.height && el1.y + el1.height > el2.y);
}

void checkColliding(GameState *state, Player *player)
{
    for (int i = 0; i < CACTUS_COUNT; i++)
    {
        if (state->cactus[i].is_dead)
        {
            continue;
        }
        Rectangle playerCord = {
            .x = player->pos.x + 25,
            .y = player->pos.y,
            .width = player->size.x - 25,
            .height = player->size.y};
        Rectangle cactusCord = {
            .x = state->cactus[i].pos.x,
            .y = state->cactus[i].pos.y,
            .width = state->cactus[i].size.x,
            .height = state->cactus[i].size.y};
        if (isColliding(playerCord, cactusCord))
        {
#if GAME_DEBUG
            DrawRectangle(0, 0, 400, 400, RED);
#endif
            if (state->score > state->high_score)
            {
                state->high_score = state->score;
            }
            state->status = END;
        }
    }
}

GameState init_game(int high_score)
{
    GameState state = {0};
    state.factor = GetScreenHeight() / 12;
    state.y = state.factor * 3;
    state.height = state.factor * 5;
    state.ground_pos = state.y + state.height;
    state.width = GetScreenWidth();
    state.speed = 7;
    state.status = START;
    state.high_score = high_score;
    for (int i = 0; i < CACTUS_COUNT; i++)
    {
        state.cactus[i] = create_cactus(&state);
    }
    return state;
}

Player init_player(GameState *state, Texture texture)
{
    Player player = {
        .size = (Vector2){.x = (texture.width / 8) * 3, .y = texture.height * 3},
        .init_pos = (Vector2){60, state->ground_pos - texture.height * 3},
        .yd = 0,
        .speed = 8,
        .texture = texture,
        .frame_rec = {0.0f, 0.0f, (float)texture.width / 8, (float)texture.height},
        .current_frame = 0,
        .frames_counter = 0,
        .frames_speed = 8,
        .frames_count = 8,
    };

    player.pos = player.init_pos;
    return player;
}

void render_start(GameState *state, StartAnimation *start_animation)
{
    char *text = "Press To Start The Game.";
    int font_size = 60;
    int text_width = MeasureText(text, font_size);
    DrawText(text, state->width / 2 - text_width / 2, state->ground_pos - 100, font_size, GRAY);
    if (GetGestureDetected() == GESTURE_TAP)
    {
        state->status = PLAYING;
    }
    float w = start_animation->frame_rec.width * 6;
    float h = start_animation->frame_rec.height * 6;

    start_animation->frames_counter++;
    if (start_animation->frames_counter >= (60 / start_animation->frames_speed))
    {
        start_animation->frames_counter = 0;
        start_animation->current_frame++;
        if (start_animation->current_frame > start_animation->frames_count - 1)
            start_animation->current_frame = 0;
        start_animation->frame_rec.x = (float)start_animation->current_frame * (float)start_animation->texture.width / start_animation->frames_count;
    }
    DrawTexturePro(
        start_animation->texture,
        start_animation->frame_rec,
        (Rectangle){.x = state->width / 2 - w / 2, .y = (GetScreenHeight() / 2 - h / 2) - 100, .height = h, .width = w},
        (Vector2){0, 0}, 0,
        WHITE);
    render_map(state);
}

void render_score(GameState *state)
{
    int fs = 50;
    char score_str[10];
    snprintf(score_str, sizeof(score_str), "%d", state->score);
    int font_width = MeasureText(score_str, fs);
    DrawText(score_str, state->width - 20 - font_width, state->y + 50, fs, GRAY);

    char hstr[10];
    snprintf(hstr, sizeof(hstr), "%d", state->high_score);
    int h_width = MeasureText(hstr, fs);
    DrawText(hstr, state->width - 60 - font_width - h_width, state->y + 50, fs, GRAY);
}
void render_playing(GameState *state, Player *player)
{

    update_player(player);
    update_cactus(state);
    double time_ms = GetTime() * 1000;
    if (time_ms - state->last_draw_time > 2000 + GetRandomValue(0, 3000))
    {
        __android_log_print(ANDROID_LOG_INFO, "INFO", "%f", time_ms - state->last_draw_time);
        state->last_draw_time = time_ms;
        for (int i = 0; i < CACTUS_COUNT; i++)
        {
            if (state->cactus[i].is_dead)
            {
                state->cactus[i] = create_cactus(state);
                break;
            }
        }
    }
    if (time_ms - state->last_score_time > 100)
    {
        state->last_score_time = time_ms;
        state->score += 1;
    }
    // if (state->score == 200)
    // {
    //     state->speed += 1;
    // }
    checkColliding(state, player);
    render_score(state);
    render_map(state);
    render_cactus(state);
    render_player(state, player);
}

void render_end(GameState *state, Player *player)
{
    char *text = "GAME OVER!";
    int font_size = 60;
    int text_width = MeasureText(text, font_size);
    DrawText(text, state->width / 2 - text_width / 2, state->ground_pos - 100, font_size, GRAY);
    render_map(state);
    render_score(state);
    render_cactus(state);
    render_player(state, player);
}

int main(void)
{
    InitWindow(0, 0, "deno");
    SetTargetFPS(60);

    Image player_image = LoadImage("DinoSprites - doux.png");
    if (player_image.data == NULL)
    {
        return 1;
    }
    // ImageResize(&player_image, player_image.width, player_image.height);
    Texture player_texture = LoadTextureFromImage(player_image);
    UnloadImage(player_image);
    char *image_path[4] = {
        "Rock Pile 1 - WHITE - BIG.PNG",
        "Rock Pile 7 - SILVER - BIG.PNG",
        "Rock Pile 8 - ORANGE - BIG.PNG",
        "Rock Pile 12 - BEIGE - BIG.PNG",
    };
    for (int i = 0; i < 4; i++)
    {
        Image image = LoadImage(image_path[i]);
        cactus_textures[i] = LoadTextureFromImage(image);
        UnloadImage(image);
    }

    Image start_animation_image = LoadImage("64X128_Runing_Free.png");
    StartAnimation start_animation = {0};
    UnloadImage(start_animation_image);

    start_animation.texture = LoadTextureFromImage(start_animation_image);
    start_animation.frame_rec = (Rectangle){0.0f, 0.0f, (float)start_animation.texture.width / 8, (float)start_animation.texture.height};
    start_animation.current_frame = 0;
    start_animation.frames_counter = 0;
    start_animation.frames_speed = 8;
    start_animation.frames_count = 8;

    GameState state = init_game(0);
    Player player = init_player(&state, player_texture);
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(WHITE);
        // DrawTexture(playerTexture, 100, 100, RED);
        switch (state.status)
        {
        case START:
            render_start(&state, &start_animation);
            break;
        case PLAYING:
            render_playing(&state, &player);
            break;
        case END:
        {
            render_end(&state, &player);
            if (GetGestureDetected() == GESTURE_TAP)
            {
                state = init_game(state.high_score);
                player = init_player(&state, player_texture);
                state.status = PLAYING;
            }
        }
        break;
        default:
            break;
        }
        EndDrawing();
    }

    return 0;
}