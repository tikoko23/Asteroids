#include <raylib.h>
#include <raymath.h>
#include "../lib/raygui.h"

#include <stdio.h>
#include <math.h>

#include "config.h"

#include "player.c"
#include "asteroid.c"
#include "projectile.c"
#include "particle.c"
#include "util.c"

#define NEARBLACK ((Color) { .a = 255, .r = 10, .g = 10, .b = 10 })

bool ShouldExit = false;
bool Paused = false;
bool Debug = false;
bool AtMenu = true;
float GlobalSoundVolume = 1.0f;
float last_shot_time;
int Score = 0;
Vector2 CameraOffset = (Vector2) { .x = 0, .y = 0 };

Sound sfx_hit;
Sound sfx_collide;
Sound sfx_explode;
Sound sfx_shoot;

void SetGlobalVolume(float volume)
{
    volume /= 5;
    SetSoundVolume(sfx_hit, volume);
    SetSoundVolume(sfx_collide, volume);
    SetSoundVolume(sfx_explode, volume);
    SetSoundVolume(sfx_shoot, volume);
}

void StartMenu()
{
    for (int i = 0; i < MAX_ASTEROID_COUNT; ++i)
    {
        SpawnedAsteroids[i].active = false;
    }

    for (int i = 0; i < MAX_PROJECTILE_COUNT; ++i)
    {
        SpawnedProjectiles[i].active = false;
    }

    for (int i = 0; i < MAX_PARTICLE_COUNT; ++i)
    {
        SpawnedParticles[i].active = false;
    }

    player_position = (Vector2) { .x = 50, .y = WINDOW_HEIGHT / 2 };
    player_rotation = 90.0f;
    player_rotation_queue = 0;
    player_forward_queue = 0;
    player_sideways_queue = 0;

    AtMenu = true;
}

void EndMenu()
{
    AtMenu = false;

    for (int i = 0; i < MAX_ASTEROID_COUNT; ++i)
    {
        SpawnedAsteroids[i].active = false;
    }

    for (int i = 0; i < MAX_PROJECTILE_COUNT; ++i)
    {
        SpawnedProjectiles[i].active = false;
    }

    for (int i = 0; i < MAX_PARTICLE_COUNT; ++i)
    {
        SpawnedParticles[i].active = false;
    }

    Score = 0;

    player_health = player_max_health;
    player_position = (Vector2) { .x = WINDOW_WIDTH / 2, .y = WINDOW_HEIGHT / 2 };
    player_rotation = 0.0f;
}

void DrawSettings()
{
    float old_volume = GlobalSoundVolume;

    unsigned int text_size = MeasureText("VOLUME", 14);
    DrawText(
        "VOLUME",
        (WINDOW_WIDTH - text_size) / 2,
        WINDOW_HEIGHT - (SOUND_VOLUME_SLIDER_BOTTOM_PADDING + SOUND_VOLUME_SLIDER_HEIGHT) - 16,
        14,
        GRAY
    );
    
    GuiSlider(
        (Rectangle) {
            .x = (WINDOW_WIDTH - SOUND_VOLUME_SLIDER_WIDTH) / 2,
            .y = WINDOW_HEIGHT - (SOUND_VOLUME_SLIDER_BOTTOM_PADDING + SOUND_VOLUME_SLIDER_HEIGHT),
            .width = SOUND_VOLUME_SLIDER_WIDTH,
            .height = SOUND_VOLUME_SLIDER_HEIGHT
        },
        "", "", &GlobalSoundVolume, 0.0, 2.0
    );

    if (GlobalSoundVolume != old_volume)
        SetGlobalVolume(GlobalSoundVolume);
}

void input()
{
    if (!AtMenu)
    {
        if (IsKeyDown(KEY_W))
        {
            player_forward_queue += 1;
        }
        if (IsKeyDown(KEY_A))
        {
            player_sideways_queue -= 1;
        }
        if (IsKeyDown(KEY_S))
        {
            player_forward_queue -= 1.0f * PLAYER_BACKWARDS_MULTIPLIER;
        }
        if (IsKeyDown(KEY_D))
        {
            player_sideways_queue += 1;
        }

        if (IsKeyDown(KEY_SPACE) && last_shot_time > 0.25f)
        {
            last_shot_time = 0.0f;
            Projectile projectile = {
                .position = Vector2Add(
                    player_position,
                    Vector2Scale(
                        player_direction_vector,
                        player_radius
                    )
                ),
                .move_direction = player_direction_vector,
                .speed = PROJECTILE_MOVE_SPEED,
                .color = RAYWHITE,
                .active = true
            };

            bool added = AddProjectile(&projectile);

            if (added)
            {
                SetSoundPitch(sfx_shoot, GetRandomValue(8, 12) / 10.0f);
                PlaySound(sfx_shoot);
            }

        }
        if (IsKeyPressed(KEY_ESCAPE))
        {
            Paused = !Paused;
        }
    }

    if (IsKeyPressed(KEY_F4))
    {
        Debug = !Debug;
    }

    Vector2 mouse_pos = GetMousePosition();

    if (!Paused && !AtMenu)
    {
        Vector2 player_dif = Vector2Subtract(player_position, mouse_pos);

        float angle = RAD2DEG * atan2f(player_dif.y, player_dif.x) - 90;

        player_rotation = angle;
    }
}

void step(float delta)
{
    player_rotation += player_target_rotation_offset * delta * 5; //player_rotation_queue * delta * PLAYER_ROTATE_SPEED;
    player_rotation = fmodf(player_rotation, 360.0f);

    player_direction_vector = Vector2Normalize((Vector2) {
        .x = sinf(player_rotation * DEG2RAD),
        .y = -cosf(player_rotation * DEG2RAD)
    });

    if (player_rotation > 360.0f)
        player_rotation = fmod(player_rotation, 360.0f);

    Vector2 v_forward = Vector2Scale(
        player_direction_vector,
        delta * PLAYER_SPEED * player_forward_queue
    );

    Vector2 v_sideways = Vector2Scale(
        Vector2Rotate(player_direction_vector, DEG2RAD * 90),
        delta * PLAYER_SPEED * PLAYER_SIDEWAYS_MULTIPLIER * player_sideways_queue
    );

    Vector2 v_full = Vector2Add(v_forward, v_sideways);

    player_position = Vector2Add(player_position, v_full);

    last_shot_time += delta;

    player_forward_queue *= PLAYER_SPEED_DAMPEN;
    player_sideways_queue *= PLAYER_SPEED_DAMPEN;
    player_rotation_queue *= PLAYER_ROTATION_DAMPEN;
    CameraOffset = Vector2Scale(CameraOffset, -0.9);

    TickAsteroids(delta);
    TickProjectiles(delta);
    TickParticles(delta);

    for (int i = 0; i < MAX_ASTEROID_COUNT; ++i)
    {
        Asteroid* asteroid = &SpawnedAsteroids[i];

        if (!asteroid->active)
            continue;

        for (int j = 0; j < MAX_PROJECTILE_COUNT; ++j)
        {
            Projectile* projectile = &SpawnedProjectiles[j];

            if (!projectile->active)
                continue;

            bool collides = CheckCollisionPointCircle(projectile->position, asteroid->position, asteroid->inner_radius);

            if (collides)
            {
                bool destroyed = ProcessAsteroidHit(asteroid, projectile);
                SpawnParticles(5, WHITE, projectile->position, 5, asteroid->speed / 10, Vector2Negate(projectile->move_direction), 80.0f);
                DestroyProjectile(projectile);

                if (destroyed)
                {
                    SetSoundPitch(sfx_collide, GetRandomValue(8, 12) / 10.0f);
                    PlaySound(sfx_collide);
                }
                else {
                    SetSoundPitch(sfx_hit, GetRandomValue(8, 12) / 10.0f);
                    PlaySound(sfx_hit);
                }
            }
        }

        bool collides_player = CheckCollisionCircles(player_position, player_inner_radius, asteroid->position, asteroid->inner_radius);

        if (collides_player)
        {
            // ShouldExit = true;
            player_health -= asteroid->sides * 3;
            DestroyAsteroid(asteroid);
            SetSoundPitch(sfx_explode, GetRandomValue(8, 12) / 10.0f);
            PlaySound(sfx_explode);

            if (player_health <= 0)
                StartMenu();
        }
    }
}

void draw()
{
    BeginDrawing();

    ClearBackground(NEARBLACK);

    // Draw player
    DrawPoly(Vector2Add(CameraOffset, player_position), 3, player_radius, player_rotation - 90, PLAYER_COLOR);

    Vector2 edge_offset = Vector2Scale(player_direction_vector, player_radius);

    DrawLineEx(
        Vector2Add(CameraOffset, player_position),
        Vector2Add(Vector2Add(CameraOffset, player_position), edge_offset),
        2, PLAYER_FORWARD_LINE_COLOR
    );

    // Draw asteroids

    for (int i = 0; i < MAX_ASTEROID_COUNT; ++i)
    {
        Asteroid* asteroid = &SpawnedAsteroids[i];

        if (!asteroid->active)
            continue;

        if (Debug)
            DrawCircle(asteroid->position.x, asteroid->position.y, asteroid->inner_radius, LIME);

        DrawPolyLines(
            Vector2Add(CameraOffset, asteroid->position),
            asteroid->sides,
            (float) asteroid->size,
            asteroid->rotation,
            asteroid->color
        );
    }

    // Draw projectiles

    for (int i = 0; i < MAX_PROJECTILE_COUNT; ++i)
    {
        Projectile* projectile = &SpawnedProjectiles[i];

        if (!projectile->active)
            continue;

        Vector2 offset = Vector2Scale(projectile->move_direction, -PROJECTILE_VISUAL_LENGTH);

        DrawLineEx(
            Vector2Add(CameraOffset, projectile->position),
            Vector2Add(CameraOffset, Vector2Add(projectile->position, offset)),
            3,
            projectile->color
        );
    }

    // Draw particles

    for (int i = 0; i < MAX_PARTICLE_COUNT; ++i)
    {
        Particle* particle = &SpawnedParticles[i];

        if (!particle->active)
            continue;

        DrawRectanglePro(
            (Rectangle) {
                .height = particle->size,
                .width = particle->size,
                .x = particle->position.x + CameraOffset.x,
                .y = particle->position.y + CameraOffset.y
            },
            Vector2Zero(),
            particle->rotation,
            particle->color
        );
    }

    // Draw text
    if (Debug)
    {
        unsigned int length = MeasureText("DEBUG", 20);
        DrawText("DEBUG", WINDOW_WIDTH - (length + 20), 20, 20, LIME);
        DrawText(TextFormat("Player pos: %.0f, %.0f", player_position.x - WINDOW_WIDTH / 2, player_position.y - WINDOW_HEIGHT / 2), 20, 45, 20, LIME);
        DrawFPS(20, 20);
    }
    
    if (AtMenu)
    {

        unsigned int length = MeasureText("ASTEROIDS", 54);
        DrawText("ASTEROIDS", (WINDOW_WIDTH - length) / 2, 80, 54, WHITE);

        DrawSettings();

        if (GuiButton(
            (Rectangle) {
                .x = (WINDOW_WIDTH - START_BUTTON_WIDTH) / 2,
                .y = (WINDOW_HEIGHT - (START_BUTTON_BOTTOM_PADDING + START_BUTTON_HEIGHT)),
                .width = START_BUTTON_WIDTH,
                .height = START_BUTTON_HEIGHT
            },
            "START"
        ))
        {
            EndMenu();
        }

        EndDrawing();
        return;
    }

    // Draw bottom bar

    {
        DrawRectangle(0, WINDOW_HEIGHT - BOTTOM_BAR_HEIGHT, WINDOW_WIDTH, BOTTOM_BAR_HEIGHT, BOTTOM_BAR_COLOR);
        DrawLineEx(
            (Vector2) { .x = 0, .y = WINDOW_HEIGHT - BOTTOM_BAR_HEIGHT },
            (Vector2) { .x = WINDOW_WIDTH, .y = WINDOW_HEIGHT - BOTTOM_BAR_HEIGHT },
            BOTTOM_BAR_LINE_THICKNESS,
            BOTTOM_BAR_LINE_COLOR
        );
    }

    // Draw score
    {
        static unsigned int font_size = 24;
        const char* score_text = TextFormat("Score: %d", Score);

        DrawText(score_text, 10, WINDOW_WIDTH - font_size - 2, font_size, WHITE);
    }

    // Draw health bar
    {
        float percent = player_health / player_max_health;
        float empty_percent = 1.0f - percent;

        Color color = WHITE;

        if (percent > 0.9f)
            color = GREEN;
        else if (percent > 0.7f)
            color = (Color) { .a = 255, .r = 80, .g = 240, .b = 20 };
        else if (percent > 0.5f)
            color = YELLOW;
        else if (percent > 0.25f)
            color = ORANGE;
        else
            color = RED;

        DrawRectangle(
            WINDOW_WIDTH - (HEALTH_BAR_PADDING + HEALTH_BAR_WIDTH),
            WINDOW_HEIGHT - (HEALTH_BAR_PADDING + HEALTH_BAR_THICKNESS),
            HEALTH_BAR_WIDTH,
            HEALTH_BAR_THICKNESS,
            BLACK
        );

        DrawRectangle(
            WINDOW_WIDTH - (HEALTH_BAR_PADDING + HEALTH_BAR_WIDTH) + HEALTH_BAR_WIDTH * empty_percent,
            WINDOW_HEIGHT - (HEALTH_BAR_PADDING + HEALTH_BAR_THICKNESS),
            HEALTH_BAR_WIDTH * percent,
            HEALTH_BAR_THICKNESS,
            color
        );
    }

    // Draw pause screen

    if (Paused)
    {
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color) { .a = 180, .r = 0, .g = 0, .b = 0 });

        const char* text = "PAUSED";
        unsigned int length = MeasureText(text, 54);

        DrawText(text, (WINDOW_WIDTH - length) / 2, 50, 54, WHITE);
        DrawSettings();
    }

    EndDrawing();
}

int main()
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Asteroids");
    InitAudioDevice();

    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    float asteroid_timer_elapsed = 0.0f;
    float asteroid_timer_target = 1.0f;

    score_counter = &Score;
    camera_offset = &CameraOffset;

    sfx_collide = LoadSound("assets/sound/collide.wav");
    sfx_shoot = LoadSound("assets/sound/shoot.wav");
    sfx_hit = LoadSound("assets/sound/hit.wav");
    sfx_explode = LoadSound("assets/sound/explode.wav");

    SetSoundVolume(sfx_collide, 0.2f);
    SetSoundVolume(sfx_shoot, 0.2f);
    SetSoundVolume(sfx_hit, 0.2f);
    SetSoundVolume(sfx_explode, 0.2f);

    player_inner_radius = GetInnerRadius(3, player_radius);

    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, 0x00000000);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xFFFFFFFF);
    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, 0xFFFFFF10);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 36);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 5);

    StartMenu();

    float timer_5s_elapsed = 5.0f;
    float timer_1s_elapsed = 1.0f;

    while (!WindowShouldClose() && !ShouldExit) {

        float delta = 0;

        input();

        if (!Paused)
        {
            delta = GetFrameTime();
            step(delta);
        }

        asteroid_timer_elapsed += delta;
        timer_5s_elapsed += delta;
        timer_1s_elapsed += delta;

        if (asteroid_timer_elapsed >= asteroid_timer_target && !AtMenu)
        {
            asteroid_timer_elapsed = 0.0f;
            asteroid_timer_target = ((float) GetRandomValue(ASTEROID_DELAY_MIN * 10, ASTEROID_DELAY_MAX * 10)) / 10.0f;
            asteroid_timer_target /= Clamp(Score / 1000, 1, 10);

            printf("New asteroid spawning...\n");

            Vector2 pos = Vector2Zero();

            switch(GetRandomValue(1, 4))
            {
                case 1:
                    // Left
                    printf("left\n");
                    pos.x = -ASTEROID_SPAWN_DISTANCE;
                    pos.y = GetRandomValue(-ASTEROID_SPAWN_DISTANCE, ASTEROID_SPAWN_DISTANCE + WINDOW_HEIGHT);
                    break;

                case 2:
                    // Up
                    printf("up\n");
                    pos.y = -ASTEROID_SPAWN_DISTANCE;
                    pos.x = GetRandomValue(-ASTEROID_SPAWN_DISTANCE, ASTEROID_SPAWN_DISTANCE + WINDOW_WIDTH);
                    break;

                case 3:
                    // Right
                    printf("right\n");
                    pos.x = WINDOW_WIDTH + ASTEROID_SPAWN_DISTANCE;
                    pos.y = GetRandomValue(-ASTEROID_SPAWN_DISTANCE, ASTEROID_SPAWN_DISTANCE + WINDOW_HEIGHT);
                    break;

                case 4:
                    // Down
                    printf("down\n");
                    pos.y = WINDOW_HEIGHT + ASTEROID_SPAWN_DISTANCE;
                    pos.x = GetRandomValue(-ASTEROID_SPAWN_DISTANCE, ASTEROID_SPAWN_DISTANCE + WINDOW_WIDTH);
                    break;
            }

            printf("%.0f, %.0f\n", pos.x, pos.y);

            Vector2 direction = Vector2Zero();

            float randX = GetRandomValue(-ASTEROID_MAX_CENTER_OFFSET * 10, ASTEROID_MAX_CENTER_OFFSET * 10) / 10.0f;
            float randY = GetRandomValue(-ASTEROID_MAX_CENTER_OFFSET * 10, ASTEROID_MAX_CENTER_OFFSET * 10) / 10.0f;

            direction = Vector2Subtract((Vector2) { .x = WINDOW_WIDTH / 2 + randX, .y = WINDOW_HEIGHT / 2 + randY }, pos);

            printf("direction: %.5f, %.5f\n", direction.x, direction.y);

            int size = GetRandomValue(ASTREOID_MIN_SIZE, ASTEROID_MAX_SIZE);

            Asteroid asteroid = {
                .position = pos,
                .sides = SidesFromSize(size),
                .size = size,
                .move_direction = Vector2Normalize(direction),
                .speed = GetRandomValue(ASTEROID_MIN_SPEED * 10, ASTEROID_MAX_SPEED * 10) / 10.0f,
                .rotation = 0.0f,
                .rotation_speed = GetRandomValue(ASTEROID_MIN_ROTATION_SPEED * 10, ASTEROID_MAX_ROTATION_SPEED * 10) / 10.0f,
                .color = PINK,
                .active = true
            };

            SetInnerRadius(&asteroid);

            AddAsteroid(&asteroid);

            printf("New timer delay: %.2f\n", asteroid_timer_target);
        }

        if (timer_5s_elapsed >= 5.0f)
        {
            if (AtMenu)
            {
                int size = GetRandomValue(30, 80);

                Asteroid asteroid = {
                    .position = (Vector2) { .x = WINDOW_WIDTH + 100, .y = WINDOW_HEIGHT / 2 },
                    .move_direction = (Vector2) { .x = -1, .y = 0 },
                    .rotation_speed = 50.0f,
                    .speed = 10000 / size,
                    .size = size,
                    .sides = SidesFromSize(size),
                    .color = PINK,
                    .active = true
                };

                SetInnerRadius(&asteroid);
                AddAsteroid(&asteroid);
            }

            timer_5s_elapsed = 0.0f;
        }

        if (timer_1s_elapsed >= 1.0f)
        {
            if (AtMenu)
            {
                Projectile projectile = {
                    .position = Vector2Add(
                        player_position,
                        Vector2Scale(
                            player_direction_vector,
                            player_radius
                        )
                    ),
                    .move_direction = player_direction_vector,
                    .speed = PROJECTILE_MOVE_SPEED,
                    .color = RAYWHITE,
                    .active = true
                };

                bool added = AddProjectile(&projectile);

                if (added)
                {
                    SetSoundPitch(sfx_shoot, GetRandomValue(8, 12) / 10.0f);
                    PlaySound(sfx_shoot);
                }
            }

            timer_1s_elapsed = 0.0f;
        }

        draw();
    }

    CloseWindow();

    return 0;
}