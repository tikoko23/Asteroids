#ifndef _PROJECTILE_C
#define _PROJECTILE_C

#include <raylib.h>
#include <raymath.h>

#include "config.h"
#include "projectile.h"

Projectile SpawnedProjectiles[MAX_PROJECTILE_COUNT] = {};

bool AddProjectile(Projectile* projectile)
{
    for (int i = 0; i < MAX_PROJECTILE_COUNT; ++i)
    {
        if (!SpawnedProjectiles[i].active)
        {
            SpawnedProjectiles[i] = *projectile;
            return true;
        }
    }

    return false;
}

void TickProjectiles(double delta)
{
    for (int i = 0; i < MAX_PROJECTILE_COUNT; ++i)
    {
        Projectile* projectile = &SpawnedProjectiles[i];

        if (!projectile->active)
            continue;

        projectile->position = Vector2Add(
            projectile->position,
            Vector2Scale(projectile->move_direction, delta * projectile->speed)
        );

        if (
            projectile->position.x <= -PROJECTILE_DESPAWN_DISTANCE ||
            projectile->position.x >= WINDOW_WIDTH + PROJECTILE_DESPAWN_DISTANCE ||
            projectile->position.y <= -PROJECTILE_DESPAWN_DISTANCE ||
            projectile->position.y >= WINDOW_HEIGHT + PROJECTILE_DESPAWN_DISTANCE
        ) {
            projectile->active = false;
            continue;
        }
    }
}

void DestroyProjectile(Projectile* projectile)
{
    projectile->active = false;
}

#endif