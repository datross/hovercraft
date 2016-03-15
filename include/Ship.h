#ifndef SHIP_H
#define SHIP_H

#include <Vec.h>

typedef struct {
    Vec2 pos, vel, accel;
    Vec2 size; /* Utilisée pour le quad texturé et les collisions.
                  Normalement la taille de référence est : une unité. */
    float tilt;
    float accel_multiplier;
    float tilt_step;
    float friction;
    float max_speed;
    GLuint texture_id;
} Ship;

void Ship_render(const Ship *s);

#endif /* SHIP_H */
