#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <GL/gl.h>
#include <Map.h>
#include <Utils.h>
#include <Sprite.h>

void MapData_free(MapData * m) {
    glDeleteTextures(1, &m->terrain.tex_id);
    glDeleteTextures(1, &m->artwork.tex_id);
    for(; m->wall_count; --m->wall_count) {
        ConvexShape_free_content(&(m->walls[m->wall_count].physic_obstacle.shape));
    }
    free(m->walls);
    if(m->music) Mix_FreeMusic(m->music);
}

static void Map_renderCheckpoint(const Checkpoint *c) {
    glPushMatrix();
    {
        glTranslatef(c->pos.x, c->pos.y, 0.f);
        glScalef(c->radius, c->radius, 1.f);
        renderCircle(64, false);
    }
    glPopMatrix();
}

void Map_renderWalls(const Map *m) {
    GLfloat col[4];
    glGetFloatv(GL_CURRENT_COLOR, col);
    
    for(unsigned i = 0; i < m->data->wall_count; ++i) {
        glColor3f(m->data->walls[i].color.r,
                  m->data->walls[i].color.g,
                  m->data->walls[i].color.b);
        glPushMatrix();
        glTranslatef(m->data->walls[i].physic_obstacle.position.x,
             m->data->walls[i].physic_obstacle.position.y, 0.f);
        glRotatef(m->data->walls[i].physic_obstacle.rotation, 0.f, 0.f, 1.f);
        if(m->data->walls[i].physic_obstacle.shape.type == POLYGON) {
            glBegin(GL_POLYGON);
            for(unsigned j = 0; j < m->data->walls[i].physic_obstacle.shape.shape.polygon.nb_vertices; ++j) {
                glVertex2f(m->data->walls[i].physic_obstacle.shape.shape.polygon.vertices[j].x,
                           m->data->walls[i].physic_obstacle.shape.shape.polygon.vertices[j].y);
            }
            glEnd();
        } else {
            glScalef(m->data->walls[i].physic_obstacle.shape.shape.circle.radius,
                     m->data->walls[i].physic_obstacle.shape.shape.circle.radius, 1.f);
            renderDisk(32 * ((unsigned)m->data->walls[i].physic_obstacle.shape.shape.circle.radius + 1));
        }
        glPopMatrix();
    }
    glColor4f(col[0], col[1], col[2], col[3]);
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
    Map_renderWalls(m);
    renderAxes();
}
