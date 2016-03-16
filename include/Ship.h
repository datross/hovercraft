#ifndef SHIP_H
#define SHIP_H

#include <Vec.h>

typedef struct {
    Vec2  pos; /* Ciblage d'objectif. */
    Vec2  scale;
    float distance; /* privee. */
    float theta;    /* privee. */
    float r,g,b;
} ShipGuide;

typedef struct {
    Vec2 pos, vel, accel;
    Vec2 size; /* Utilisée pour le quad texturé et les collisions.
                  Normalement la taille de référence est : une unité. */
    ShipGuide guides[5];
    size_t guide_count;
    float tilt;
    float accel_multiplier;
    float tilt_step;
    float friction;
    float max_speed;
    float r,g,b; /* Temporaire. sera remplacé bientot. */
    size_t next_checkpoint_index;
} Ship;

void Ship_render(const Ship *s);
void Ship_renderGuides(const Ship *s);
void Ship_renderBoundingVolumes(const Ship *s);

#endif /* SHIP_H */
