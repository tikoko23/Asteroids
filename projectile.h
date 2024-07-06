#ifndef _PROJECTILE_H
#define _PROJECTILE_H

#include <raylib.h>

#include "config.h"

typedef struct {
    Vector2 position;
    Vector2 move_direction;
    float speed;
    Color color;
    bool active;
} Projectile;

Projectile SpawnedProjectiles[MAX_PROJECTILE_COUNT];

bool AddProjectile(Projectile* projectile);
void TickProjectiles(double delta);
void DestroyProjectile(Projectile* projectile);

#endif