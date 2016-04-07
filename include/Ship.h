#ifndef SHIP_H
#define SHIP_H

#include <Vec.h>
#include <Physics.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Vec2  pos; /* Ciblage d'objectif. */
    Vec2  scale;
    float distance; /* privee. */
    float theta;    /* privee. */
    float r,g,b;
} ShipGuide;

typedef struct {
    Solid physic_solid;
    Vec2 size; /* Utilisée pour le quad texturé et les collisions.
                  Normalement la taille de référence est : une unité. */
    ShipGuide guides[5];
    size_t guide_count;
    float accel_multiplier;
    float tilt_step;
    float friction;
    float max_speed;
    float r,g,b; /* Temporaire. sera remplacé bientot. */
    size_t next_checkpoint_index;
    Force main_translate_force;
    Force main_rotate_force;
    Force main_translation_friction;
    Force main_rotation_friction;
} Ship;

void Ship_init(Ship *s);
void Ship_deinit(Ship *s);
void Ship_render(const Ship *s);
void Ship_renderGuides(const Ship *s);
void Ship_renderBoundingVolumes(const Ship *s);

#endif /* SHIP_H */
