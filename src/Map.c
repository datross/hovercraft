#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <GL/gl.h>
#include <Map.h>
#include <Utils.h>
#include <Sprite.h>

static void Map_renderCheckpoint(const Checkpoint *c) {
    glPushMatrix();
    {
        glTranslatef(c->pos.x, c->pos.y, 0.f);
        glScalef(c->radius, c->radius, 1.f);
        renderCircle(64, false);
    }
    glPopMatrix();
}

void Map_renderCheckpoints(const Map *m, size_t next_checkpoint_index) {
    size_t i;
    const Color3 *co = &m->data->checkpoint_color;
    const Color3 *hi = &m->data->checkpoint_highlight;
    glColor3f(hi->r, hi->g, hi->b);
    for(i=0 ; i<next_checkpoint_index ; ++i)
        Map_renderCheckpoint(m->data->checkpoints+i);
    glColor3f(co->r, co->g, co->b);
    for(    ; i<m->data->checkpoint_count ; ++i)
        Map_renderCheckpoint(m->data->checkpoints+i);
}
void Map_render(const Map *m) {
    TiledQuad_render(&m->data->terrain);
    renderAxes();
}
