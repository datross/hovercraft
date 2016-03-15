#include <stdio.h>
#include <GL/gl.h>
#include <Ship.h>
#include <Utils.h>

static void Ship_renderGuide(const Ship *s) {
    glColor3f(1.f,.0f,0.f);
    glPushMatrix(); 
    {
        glTranslatef(s->pos.x, s->pos.y, 0);
        glRotatef(s->objective_theta,0,0,1);
        glTranslatef(0, 1.5f, 0);
        glScalef(.6f, .6f, 1.f);
        renderIsocelesTriangle(true);
    } 
    glPopMatrix();
}

static void Ship_renderBoundingVolumes(const Ship *s) {
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
void Ship_render(const Ship *s) {
    Ship_renderBoundingVolumes(s);
    Ship_renderGuide(s);
}
