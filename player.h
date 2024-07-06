#ifndef _PLAYER_H
#define _PLAYER_H

#include <raylib.h>

Vector2 player_position;
Vector2 player_direction_vector;
float player_rotation;
float player_target_rotation_offset;
float player_radius;
float player_forward_queue;
float player_sideways_queue;
float player_rotation_queue;
float player_inner_radius;
float player_health;
float player_max_health;

#endif