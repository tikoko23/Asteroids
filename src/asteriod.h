#ifndef _ASTEROID_H
#define _ASTEROID_H

#include <raylib.h>

#include "config.h"
#include "projectile.h"

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

int* score_counter;

Asteroid SpawnedAsteroids[MAX_ASTEROID_COUNT];

void AddAsteroid(Asteroid* asteroid);
void TickAsteroids(double delta);
void SetInnerRadius(Asteroid* asteroid);
void DestroyAsteroid(Asteroid* asteroid);
bool ProcessAsteroidHit(Asteroid* asteroid, Projectile* proj);

#endif