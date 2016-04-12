#ifndef SPRITE_H
#define SPRITE_H

#include <GL/gl.h>
#include <Vec.h>

typedef struct {
    Vec2 rect_pos, rect_size;
    GLuint tex_id;
} Sprite;

static inline void Sprite_render(const Sprite *s) {
    glBindTexture(GL_TEXTURE_2D, s->tex_id);
    glPushMatrix();
    {
        glBegin(GL_QUADS);
        glTexCoord2f(s->rect_pos.x, s->rect_pos.y);
        glVertex2f(-.5,  .5);
        glTexCoord2f(s->rect_pos.x + s->rect_size.x, s->rect_pos.y);
        glVertex2f( .5,  .5);
        glTexCoord2f(s->rect_pos.x, s->rect_pos.y + s->rect_size.y);
        glVertex2f(-.5, -.5);
        glTexCoord2f(s->rect_pos.x + s->rect_size.x, 
                     s->rect_pos.y + s->rect_size.y);
        glVertex2f( .5, -.5);
        glEnd();
    }
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
}

#endif /* SPRITE_H */
