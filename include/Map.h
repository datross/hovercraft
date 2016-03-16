#ifndef MAP_H
#define MAP_H

#include <Vec.h>

typedef struct {
    Vec2 pos;
    float radius;
} Checkpoint;

#define MAX_CHECKPOINT_COUNT 32

typedef struct {
    Vec2 size; /* Bords, en coordonnées monde. */
    float friction;
    Checkpoint checkpoints[MAX_CHECKPOINT_COUNT];
    size_t checkpoint_count;
    /* TODO checkpoints, obstacles, etc. */
} Map;

void Map_render(const Map *m);
void Map_renderCheckpoints(const Map *m, size_t next_checkpoint_index);

#endif /* MAP_H */
