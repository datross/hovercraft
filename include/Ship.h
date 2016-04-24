#ifndef SHIP_H
#define SHIP_H

#include <Vec.h>
#include <Physics.h>
#include <Sprite.h>
#include <GameLimits.h>
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
    float accel_multiplier;
    float tilt_step;
    float friction;
    float max_speed;
    float r,g,b; /* Temporaire. sera remplacé bientot. */
    Sprite banner[MAX_PALETTES];
    Sprite side[MAX_PALETTES];
    Sprite icon[MAX_PALETTES];
    Sprite artwork[MAX_PALETTES];
    Sprite above[10][MAX_PALETTES];
} ShipData;

/* On distingue une _occurrence_ de vaisseau de ses _données_chargées_. 
 * Tous les bateaux du meme nom ont les memes caractéristiques, mais
 * chaque occurrence a un état qui lui est propre. */
typedef struct {
    const ShipData *data;
    unsigned palette_index : 3;
    unsigned above_index : 4; /* Quel est le sprite "above" courant. */
    unsigned reserved : 1;
    Solid physic_solid;
    ShipGuide guides[5];
    size_t guide_count;
    size_t next_checkpoint_index;
    Force main_translate_force;
    Force main_rotate_force;
    Force main_translation_friction;
    Force main_rotation_friction;
} Ship;

void Ship_initPhysics(Ship *s);
void Ship_deinitPhysics(Ship *s);
void Ship_render(const Ship *s);
void Ship_renderGuides(const Ship *s);
void Ship_renderBoundingVolumes(const Ship *s);
void Ship_refreshGuides(Ship *s);

#endif /* SHIP_H */
