#ifndef _PARTICLE_C
#define _PARTICLE_C

#include <raylib.h>
#include <raymath.h>

#include "config.h"
#include "particle.h"

Particle SpawnedParticles[MAX_PARTICLE_COUNT] = {};

static void AddParticle(Particle* particle)
{
    for (int i = 0; i < MAX_PARTICLE_COUNT; ++i)
    {
        if (!SpawnedParticles[i].active)
        {
            SpawnedParticles[i] = *particle;
            return;
        }
    }

    printf("Particles full\n");
}

void SpawnParticles(int amount, Color color, Vector2 pos, float size, float speed_mul, Vector2 move_direction, float max_spread_angle)
{
    for (int i = 0; i < amount; ++i)
    {
        float spread_angle = GetRandomValue(0, max_spread_angle) - max_spread_angle / 2;

        Vector2 actual_move_direction = Vector2Rotate(move_direction, DEG2RAD * spread_angle);

        Particle particle = {
            .position = pos,
            .size = size,
            .move_direction = Vector2Normalize(actual_move_direction),
            .rotation = 0.0f,
            .rotation_speed = GetRandomValue(5, 90),
            .speed = GetRandomValue(40, 60) * speed_mul,
            .color = color,
            .active = true
        };

        AddParticle(&particle);
    }
}

void TickParticles(double delta)
{
    for (int i = 0; i < MAX_PARTICLE_COUNT; ++i)
    {
        Particle* particle = &SpawnedParticles[i];

        if (!particle->active)
            continue;

        particle->position = Vector2Add(particle->position, Vector2Scale(particle->move_direction, delta * particle->speed));
        particle->rotation += particle->rotation_speed * delta;
        particle->size *= 0.97f;
        particle->speed *= 0.97f;

        if (particle->size < 0.23f)
        {
            particle->active = false;
        }
    }
}

#endif