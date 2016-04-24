#include <stdlib.h>
#include <stdio.h>
#include <Sprite.h>

static inline void Sprite_buildTexCoords(Sprite *s, Vec2u rect_pos, 
                                  Vec2u rect_size, Vec2u img_size) {
    s->texcoord_tl.x =((double)rect_pos.x)/(double)img_size.x;
    s->texcoord_tl.y =((double)rect_pos.y)/(double)img_size.y;
    s->texcoord_br.x =((double)(rect_pos.x+rect_size.x))/(double)img_size.x;
    s->texcoord_br.y =((double)(rect_pos.y+rect_size.y))/(double)img_size.y;
}

void Sprite_build(Sprite *s, GLuint tex_id, Vec2u rect_pos, 
                  Vec2u rect_size, Vec2u img_size) 
{
    Sprite_buildTexCoords(s, rect_pos, rect_size, img_size);
    s->tex_id = tex_id;
    Sprite_resizeToHeight(s, 1.f);
}

void Sprite_render(const Sprite *s) {
    glColor3f(1.f, 1.f, 1.f);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, 1.f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, s->tex_id);
    glBegin(GL_QUADS);
    glTexCoord2f(s->texcoord_tl.x, s->texcoord_br.y);
    glVertex2f(-s->half_size.x, -s->half_size.y);
    glTexCoord2f(s->texcoord_br.x, s->texcoord_br.y);
    glVertex2f( s->half_size.x, -s->half_size.y);
    glTexCoord2f(s->texcoord_br.x, s->texcoord_tl.y);
    glVertex2f( s->half_size.x,  s->half_size.y);
    glTexCoord2f(s->texcoord_tl.x, s->texcoord_tl.y);
    glVertex2f(-s->half_size.x,  s->half_size.y);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}
void Sprite_bindAlpha(float alpha) {
    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
    glBlendColor(.0f, .0f, .0f, alpha);
}
static float Sprite_getAspectRatio(const Sprite *s) {
    return (s->texcoord_br.x - s->texcoord_tl.x)/(s->texcoord_br.y - s->texcoord_tl.y);
}
void Sprite_resizeToHeight(Sprite *s, float h) {
    s->half_size.y = h*.5f;
    s->half_size.x = s->half_size.y*Sprite_getAspectRatio(s);
}
void Sprite_resizeToWidth(Sprite *s, float w) {
    s->half_size.x = w*.5f;
    s->half_size.y = s->half_size.x/Sprite_getAspectRatio(s);
}
