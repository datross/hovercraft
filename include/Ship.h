#ifndef SHIP_H
#define SHIP_H

#include <Vec.h>

typedef struct {
    Vec2 pos, vel, accel;
    Vec2 size; /* Utilisée pour le quad texturé et les collisions.
                  Normalement la taille de référence est : une unité. */
    Vec2  objective; /* Ciblage d'objectif. */
    float objective_distance; /* privee. */
    float objective_theta;    /* privee. */
    float tilt;
    float accel_multiplier;
    float tilt_step;
    float friction;
    float max_speed;
    GLuint texture_id;
    float r,g,b; /* Temporaire. sera remplacé bientot. */
    size_t next_checkpoint_index;
} Ship;

void Ship_render(const Ship *s);
void Ship_renderGuide(const Ship *s);
void Ship_renderBoundingVolumes(const Ship *s);

#endif /* SHIP_H */
