#ifndef MAP_H
#define MAP_H

#include <Vec.h>
#include <Utils.h>
#include <GameLimits.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <Sprite.h>

typedef struct {
    Vec2 pos;
    float tilt;
} MapStart;

typedef struct {
    Vec2 pos;
    float radius;
} Checkpoint;

#define MAX_CHECKPOINTS 32
#define MAP_NAME_LEN 128

typedef struct {
    char name[MAP_NAME_LEN];
    Vec2 size; /* Bords, en coordonnées monde. */
    Sprite artwork, banner;
    TiledQuad terrain;
    Color3 color, checkpoint_color, checkpoint_highlight;
    float friction;
    MapStart start[MAX_PLAYERS];
    Checkpoint checkpoints[MAX_CHECKPOINTS]; 
    size_t checkpoint_count;
} MapData;

typedef struct {
    const MapData *data;
} Map;

void Map_render(const Map *m);
void Map_renderCheckpoints(const Map *m, size_t next_checkpoint_index);

#endif /* MAP_H */
