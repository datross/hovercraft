#include <stdbool.h>
#include <GL/gl.h>
#include <Map.h>
#include <Utils.h>

void Map_render(const Map *m) {
    glColor3f(0.f, .4f, .7f);
    glPushMatrix();
    {
        glScalef(2*m->size.x, 2*m->size.y, 1);
        renderSquare(true);
    }
    glPopMatrix();

    renderAxes();

    glColor3f(0.f, .7f, 1.f);
    glPushMatrix();
    {
        glTranslatef(4.f, 2.f, 0);
        renderCircle(32, false);
    }
    glPopMatrix();

}
