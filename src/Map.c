#include <stdbool.h>
#include <GL/gl.h>
#include <Map.h>
#include <Utils.h>

void Map_render(const Map *m) {
    /* TODO */
    renderAxes();

    glColor3f(0.f, .7f, 1.f);
    glPushMatrix();
    {
        glTranslatef(4.f, 2.f, 0);
        renderCircle(32, false);
    }
    glPopMatrix();
}
