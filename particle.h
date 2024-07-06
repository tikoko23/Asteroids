#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <raylib.h>

#include "config.h"

#ifndef _PARTICLE_STR
#define _PARTICLE_STR

typedef struct {
    Vector2 position;
    float size;
    Vector2 move_direction;
    float rotation;
    float rotation_speed;
    float speed;
    Color color;
    bool active;
} Particle;

#endif

Particle SpawnedParticles[MAX_PARTICLE_COUNT];

void SpawnParticles(int amount, Color color, Vector2 pos, float size, float speed_mul, Vector2 move_direction, float max_spread_angle);
void TickParticles(double delta);

#endif