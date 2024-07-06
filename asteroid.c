#ifndef _ASTEROID_C
#define _ASTEROID_C

#include <raylib.h>
#include <raymath.h>
#include <stddef.h>

#include "config.h"
#include "util.h"
#include "projectile.h"
#include "particle.h"

typedef struct {
    Vector2 position;
    int size;
    float inner_radius;
    int sides;
    Vector2 move_direction;
    float speed;
    float rotation;
    float rotation_speed;
    Color color;
    bool active;
} Asteroid;

int* score_counter = NULL;
Vector2* camera_offset = NULL;

Asteroid SpawnedAsteroids[MAX_ASTEROID_COUNT] = {};

void AddAsteroid(Asteroid* asteroid)
{
    for (int i = 0; i < MAX_ASTEROID_COUNT; ++i)
    {
        if (!SpawnedAsteroids[i].active)
        {
            SpawnedAsteroids[i] = *asteroid;
            return;
        }
    }

    printf("Asteroids full\n");
}

void TickAsteroids(double delta)
{
    for (int i = 0; i < MAX_ASTEROID_COUNT; ++i)
    {
        Asteroid* current_asteroid = &SpawnedAsteroids[i];

        if (!current_asteroid->active)
            continue;

        current_asteroid->position = Vector2Add(
            current_asteroid->position,
            Vector2Scale(
                current_asteroid->move_direction,
                current_asteroid->speed * delta
            )
        );

        if (
            current_asteroid->position.x <= -ASTEROID_DESPAWN_DISTANCE ||
            current_asteroid->position.x >= WINDOW_WIDTH + ASTEROID_DESPAWN_DISTANCE ||
            current_asteroid->position.y <= -ASTEROID_DESPAWN_DISTANCE ||
            current_asteroid->position.y >= WINDOW_HEIGHT + ASTEROID_DESPAWN_DISTANCE
        ) {
            current_asteroid->active = false;
            continue;
        }

        current_asteroid->rotation += current_asteroid->rotation_speed * delta;
    }
}

void SetInnerRadius(Asteroid* asteroid)
{
    asteroid->inner_radius = GetInnerRadius(asteroid->sides, asteroid->size);
}

void DestroyAsteroid(Asteroid* asteroid)
{
    SpawnParticles(10, PINK, asteroid->position, 20, 10, Vector2One(), 360.0f);

    if (score_counter != NULL)
        *score_counter += 10;
    
    asteroid->active = false;
    
    if (camera_offset != NULL)
    {
        *camera_offset = (Vector2) { .x = 20, .y = -10 };
    }
}

bool ProcessAsteroidHit(Asteroid* asteroid, Projectile* proj)
{
    asteroid->sides -= 1;

    if (asteroid->sides < 3)
    {
        DestroyAsteroid(asteroid);
        return true;
    }

    SpawnParticles(10, PINK, asteroid->position, 10, 10, proj->move_direction, 160.0f);

    asteroid->size = SizeFromSides(asteroid->sides);

    SetInnerRadius(asteroid);
    
    if (score_counter != NULL)
        *score_counter += 2;

    if (camera_offset != NULL)
    {
        float x_offset = GetRandomValue(-30, 30) / 10.0f;
        float y_offset = GetRandomValue(-30, 30) / 10.0f;

        *camera_offset = (Vector2) { .x = x_offset, .y = y_offset };
    }

    return false;
}

#endif