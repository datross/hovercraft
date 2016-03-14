#include <GL/gl.h>
#include <Ship.h>
#include <Utils.h>

void Ship_render(const Ship *s) {
    /* TODO */
    glPushMatrix();
    {
        glTranslatef(s->pos.x, s->pos.y, 0);
        glRotatef(s->tilt*180.f/M_PI,0,0,1);
        glColor3f(.2f,.2f,1.f);
        glPushMatrix(); 
        {
            glTranslatef(0, .5f, 0);
            glScalef(.5f, .5f, 1.f);
            renderDisk(32);
        } 
        glPopMatrix();

        glColor3f(0,0,1.f);
        renderSquare(true);
    }
    glPopMatrix();
}
