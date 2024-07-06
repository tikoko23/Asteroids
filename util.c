#ifndef _UTIL_C
#define _UTIL_C

#include <raylib.h>

float GetInnerRadius(int side_count, float corner_distance)
{
    float inner_angle = DEG2RAD * (180 - 360 / side_count);
    
    return sinf(inner_angle / 2) * corner_distance;
}

int SidesFromSize(int size)
{
    return Clamp(size / 10, 3, 9);
}

int SizeFromSides(int sides)
{
    return sides * 10;
}

float Lerpf(float l, float r, float a)
{
    return l + a * (r - l);
}

#endif