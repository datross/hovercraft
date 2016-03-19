#ifndef VEC_H
#define VEC_H

#include <stdint.h>

typedef struct { float x, y; } Vec2;
typedef struct { uint32_t x, y; } Vec2u;

float Scal2(Vec2 u, Vec2 v);

#endif /* VEC_H */
