#ifndef VEC_H
#define VEC_H

#include <stdint.h>
#include <math.h>

typedef struct { float x, y; } Vec2;
typedef struct { uint32_t x, y; } Vec2u;

Vec2 MakeVec2(float x, float y);
Vec2 AddVec2(Vec2 a, Vec2 b);
Vec2 SubVec2(Vec2 a , Vec2 b);
Vec2 MulVec2(Vec2 a, float coeff);
float Scal2(Vec2 u, Vec2 v);
float PseudoVectProd2(Vec2 a, Vec2 b);
Vec2 OrthogonalVec2(Vec2 a);

/* Fonctions de passage d'une base locale à celle globale et inversement */
Vec2 LocalToGlobal2(Vec2 local_position, Vec2 base_position, float base_angle);

#endif /* VEC_H */
