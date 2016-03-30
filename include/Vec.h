#ifndef VEC_H
#define VEC_H

#include <stdint.h>

typedef struct { float x, y; } Vec2;
typedef struct { uint32_t x, y; } Vec2u;

Vec2 MakeVec2(float x, float y);
Vec2 AddVec2(Vec2 a, Vec2 b);
Vec2 SubVec2(Vec2 a , Vec2 b);
Vec2 MulVec2(Vec2 a, float coeff);
float Scal2(Vec2 u, Vec2 v);
Vec2 OrthogonalVec2(Vec2 a);

#endif /* VEC_H */
