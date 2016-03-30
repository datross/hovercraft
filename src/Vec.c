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
