#include <Vec.h>

Vec2 MakeVec2(float x, float y) {
    Vec2 v;
    v.x = x;
    v.y = y;
    
    return v;
}

Vec2 AddVec2(Vec2 a, Vec2 b) {
    a.x += b.x;
    a.y += b.y;
    
    return a;
}

Vec2 SubVec2(Vec2 a , Vec2 b) {
    a.x -= b.x;
    a.y -= b.y;
    
    return a;
}
    
    
Vec2 MulVec2(Vec2 a, float coeff) {
    a.x *= coeff;
    a.y *= coeff;
    
    return a;
}

Vec2 OrthogonalVec2(Vec2 a) {
    float temp = a.x;
    a.x = -a.y;
    a.y = temp;
    
    return a;
}

float Scal2(Vec2 u, Vec2 v) {
    return u.x * v.x + u.y * v.y;
}

float SqrNorm(Vec2 u) {
    return u.x * u.x + u.y * u.y;
}

float PseudoVectProd2(Vec2 a, Vec2 b) {
    return a.x * b.y - a.y * b.x;
}

Vec2 Rotate2(Vec2 u, float angle) {
    float c = cosf(angle), s = sinf(angle);
    return MakeVec2(c * u.x - s * u.y, s * u.x + c * u.y);
}

Vec2 LocalToGlobal2(Vec2 local_position, Vec2 base_position, float base_angle) {
    float c = cosf(base_angle), s = sinf(base_angle);

    return AddVec2(base_position, MakeVec2( c * local_position.x - s * local_position.y, 
                                s * local_position.x + c * local_position.y));
}

Vec2 GlobalToLocal2(Vec2 global_position, Vec2 base_position, float base_angle) {
    float c = cosf(base_angle), s = sinf(base_angle);
    return MakeVec2(c * (global_position.x - base_position.x) + s * (global_position.y - base_position.y),
                    -s * (global_position.x - base_position.x) + c * (global_position.y - base_position.y));
}
