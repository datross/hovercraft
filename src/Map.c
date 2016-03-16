#include <stdbool.h>
#include <GL/gl.h>
#include <Map.h>
#include <Utils.h>

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
    glColor3f(0.f, 1.f, 0.6f);
    for(i=0 ; i<next_checkpoint_index ; ++i)
        Map_renderCheckpoint(m->checkpoints+i);
    glColor3f(1.f, 0.f, 0.f);
    for(    ; i<m->checkpoint_count ; ++i)
        Map_renderCheckpoint(m->checkpoints+i);
}
void Map_render(const Map *m) {
    renderAxes();

    glColor3f(0.f, .4f, .7f);
    glPushMatrix();
    {
        glScalef(2*m->size.x, 2*m->size.y, 1);
        renderSquare(true);
    }
    glPopMatrix();
}
