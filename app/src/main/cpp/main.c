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
    int x;
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

typedef struct
{
    Vector2 pos;
    Vector2 init_pos;
    Vector2 size;
    int yd;
    int speed;
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
    DrawRectangleV(player->pos, player->size, RED);
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
    if (player->init_pos.y - player->pos.y >= 260)
    {
        player->yd = 1 * player->speed;
    }
    if (player->init_pos.y - player->pos.y <= 0)
    {
        player->pos.y = player->init_pos.y;
        player->yd = 0;
    }
}

void update_cactus(GameState *state)
{
    for (int i = 0; i < CACTUS_COUNT; i++)
    {
        if (!state->cactus[i].is_dead)
        {
            // __android_log_print(ANDROID_LOG_INFO, "INFO", "%f", state->cactus[i].x);
            state->cactus[i].x += -1 * state->speed;
            if (state->cactus[i].x < 0)
            {
                state->cactus[i].is_dead = true;
            }
        }
    }
}

void render_cactus(GameState *state)
{

    for (int i = 0; i < CACTUS_COUNT; i++)
    {
        if (!state->cactus[i].is_dead)
        {
            // __android_log_print(ANDROID_LOG_INFO, "INFO", "%d", state->cactus[i].x);
            DrawRectangleV((Vector2){.x = state->cactus[i].x, .y = state->ground_pos - 100}, (Vector2){20, 100}, GREEN);
        }
    }
}

bool isColliding(Vector4 el1, Vector4 el2)
{
    return (el1.x < el2.x + el2.w && el1.x + el1.w > el2.x && el1.y < el2.y + el2.z && el1.y + el1.z > el2.y);
}

void checkColliding(GameState *state, Player *player)
{
    for (int i = 0; i < CACTUS_COUNT; i++)
    {
        if (state->cactus[i].is_dead)
        {
            continue;
        }
        Vector4 playerCord = {
            .x = player->pos.x,
            .y = player->pos.y,
            .w = player->size.x,
            .z = player->size.y};
        Vector4 cactusCord = {
            .x = state->cactus[i].x,
            .y = state->ground_pos - 100,
            .w = 20,
            .z = 100};
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
    state.speed = 5;
    state.status = START;
    state.high_score = high_score;
    for (int i = 0; i < CACTUS_COUNT; i++)
    {
        state.cactus[i] = (Cactus){.x = 0, .is_dead = true};
    }
    return state;
}

Player init_player(GameState *state)
{
    Player player = {
        .size = (Vector2){50, 120},
        .init_pos = (Vector2){60, state->ground_pos - 120},
        .yd = 0,
        .speed = 6,
    };

    player.pos = player.init_pos;
    return player;
}

void render_start(GameState *state)
{
    char *text = "Press To Start The Game.";
    int font_size = 60;
    int text_width = MeasureText(text, font_size);
    DrawText(text, state->width / 2 - text_width / 2, state->ground_pos - 100, font_size, GRAY);
    if (GetGestureDetected() == GESTURE_TAP)
    {
        state->status = PLAYING;
    }
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
                state->cactus[i] = (Cactus){.x = state->width + 50, .is_dead = false};
                break;
            }
        }
    }
    if (time_ms - state->last_score_time > 100)
    {
        state->last_score_time = time_ms;
        state->score += 1;
    }
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

    GameState state = init_game(0);
    Player player = init_player(&state);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(WHITE);
        switch (state.status)
        {
        case START:
            render_start(&state);
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
                player = init_player(&state);
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