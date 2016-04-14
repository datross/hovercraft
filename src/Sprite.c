#include <Sprite.h>

static inline void Sprite_buildTexCoords(Sprite *s, Vec2u rect_pos, 
                                  Vec2u rect_size, Vec2u img_size) {
    s->texcoord_tl.x = rect_pos.x/(float)img_size.x;
    s->texcoord_tl.y = rect_pos.y/(float)img_size.y;
    s->texcoord_br.x = (rect_pos.x+rect_size.x)/(float)img_size.x;
    s->texcoord_br.y = (rect_pos.y+rect_size.y)/(float)img_size.x;
}

void Sprite_build(Sprite *s, GLuint tex_id, Vec2u rect_pos, 
                  Vec2u rect_size, Vec2u img_size) 
{
    Sprite_buildTexCoords(s, rect_pos, rect_size, img_size);
    s->tex_id = tex_id;
    Sprite_resizeToHeight(s, 1.f);
}

void Sprite_render(const Sprite *s) {
    glBindTexture(GL_TEXTURE_2D, s->tex_id);
    glPushMatrix();
    {
        glBegin(GL_QUADS);
        glTexCoord2f(s->texcoord_tl.x, s->texcoord_tl.y);
        glVertex2f(-s->half_size.x,  s->half_size.y);
        glTexCoord2f(s->texcoord_br.x, s->texcoord_tl.y);
        glVertex2f( s->half_size.x,  s->half_size.y);
        glTexCoord2f(s->texcoord_tl.x, s->texcoord_br.y);
        glVertex2f(-s->half_size.x, -s->half_size.y);
        glTexCoord2f(s->texcoord_br.x, s->texcoord_br.y);
        glVertex2f( s->half_size.x, -s->half_size.y);
        glEnd();
    }
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
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
