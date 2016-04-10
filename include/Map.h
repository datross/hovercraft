#ifndef MAP_H
#define MAP_H

#include <Vec.h>
#include <GL/glu.h>
#include <GL/gl.h>

typedef struct {
    Vec2 pos;
    float radius;
} Checkpoint;

#define MAX_CHECKPOINT_COUNT 32

#define MAP_NAME_LEN 128
typedef struct {
    char name[MAP_NAME_LEN];
    GLuint texture_id;
} MapPreview;

typedef struct {
    Vec2 size; /* Bords, en coordonnées monde. */
    float friction;
    Checkpoint checkpoints[MAX_CHECKPOINT_COUNT];
    size_t checkpoint_count;
    /* TODO checkpoints, obstacles, etc. */
} Map;

void Map_render(const Map *m);
void Map_renderCheckpoints(const Map *m, size_t next_checkpoint_index);
void Map_loadFromFile(Map *m, FILE *file);
void MapPreview_loadFromFile(MapPreview *m, FILE *file);

#endif /* MAP_H */
