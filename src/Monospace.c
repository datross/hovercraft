#include <string.h>
#include <GL/gl.h>
#include <Sprite.h>
#include "Monospace_img.c"
static GLuint monospace_tex = 0;

void Monospace_setup(void) {
    glGenTextures(1, &monospace_tex);
    glBindTexture(GL_TEXTURE_2D, monospace_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, monospace_img.pixel_data);
}

void Monospace_cleanup(void) {
    glDeleteTextures(1, &monospace_tex);
    monospace_tex = 0;
}

static void Monospace_buildSprite(Sprite *s, char ascii, float height) {
    char offset = ascii-' ';
    if(offset < 0)
        offset = 0;
    static const Vec2u img_size = {1024, 1024};
    static const Vec2u rect_size = {63, 122};
    const Vec2u rect_pos = { (offset%16)*rect_size.x, (offset/16)*rect_size.y };
    Sprite_build(s, monospace_tex, rect_pos, rect_size, img_size);
    Sprite_resizeToHeight(s, height);
}

static float Monospace_getCharWidth(float height) {
    Sprite sprite;
    Monospace_buildSprite(&sprite, ' ', height);
    return 2.f*sprite.half_size.x;
}
float Monospace_getStringWidth(const char *str, float height) {
    float cw = Monospace_getCharWidth(height);
    return (strlen(str)-1)*cw;
}
/* Ne pas utiliser de scale ou rotate. */
void Monospace_renderCenteredString(const char *str, float height) {
    size_t i;
    glPushMatrix();
    float cw = Monospace_getCharWidth(height);
    glTranslatef(-Monospace_getStringWidth(str, height)*.5f, 0.f, 0.f);
    for(i=0 ; str[i] ; ++i) {
        Sprite sprite;
        Monospace_buildSprite(&sprite, str[i], height);
        Sprite_render(&sprite);
        glTranslatef(cw, 0.f, 0.f);
    }
    glPopMatrix();
}
