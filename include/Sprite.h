#ifndef SPRITE_H
#define SPRITE_H

#include <GL/gl.h>
#include <Vec.h>

typedef struct {
    /* Privé. top-left, bottom-right, du rectangle-portion de texture.  */
    Vec2 texcoord_tl, texcoord_br;
    GLuint tex_id; /* Privé. */
    /* Le seul champ public. Moitié de la taille en coordonnées monde. */
    Vec2 half_size;
} Sprite;

void Sprite_build(Sprite *s, GLuint tex_id, Vec2u rect_pos, 
                  Vec2u rect_size, Vec2u img_size);
void Sprite_render(const Sprite *s);
void Sprite_resizeToHeight(Sprite *s, float h);
void Sprite_resizeToWidth(Sprite *s, float w);

#endif /* SPRITE_H */
