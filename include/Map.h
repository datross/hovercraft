#ifndef MAP_H
#define MAP_H

#include <Vec.h>

typedef struct {
    Vec2 size; /* Bords, en coordonnées monde. */
    float friction;
    /* TODO checkpoints, obstacles, etc. */
} Map;

void Map_render(const Map *m);

#endif /* MAP_H */
