#ifndef _PLAYER_C
#define _PLAYER_C

#include <raylib.h>
#include <raymath.h>

#include "config.h"
#include "util.h"

Vector2 player_position = { .x = WINDOW_WIDTH / 2, .y = WINDOW_HEIGHT / 2 };
Vector2 player_direction_vector = (Vector2) { .x = 0, .y = 0 };
float player_rotation = 0.0f;
float player_target_rotation_offset = 0.0f;
float player_radius = 25;
float player_forward_queue = 0;
float player_sideways_queue = 0;
float player_rotation_queue = 0;
float player_inner_radius = -INFINITY;
float player_max_health = 100;
float player_health = 100;

#endif