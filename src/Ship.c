#include <stdio.h>
#include <GL/gl.h>
#include <Ship.h>
#include <Utils.h>

void Ship_renderGuides(const Ship *s) {
    int i;
    for(i=s->guide_count-1 ; i>=0 ; --i) {
        const ShipGuide *guide = s->guides + i;
        glColor3f(guide->r, guide->g, guide->b);
        glPushMatrix(); 
        {
            glTranslatef(s->pos.x, s->pos.y, 0);
            glRotatef(guide->theta,0,0,1);
            glTranslatef(0, 1.5f, 0);
            glScalef(guide->scale.x, guide->scale.y, 1.f);
            renderIsocelesTriangle(true);
        } 
        glPopMatrix();
    }
}

void Ship_renderBoundingVolumes(const Ship *s) {
    glColor3f(s->r,s->g,s->b);
    glPushMatrix();
    {
        glTranslatef(s->pos.x, s->pos.y, 0);
        glRotatef(s->tilt*180.f/M_PI,0,0,1);
        glPushMatrix(); 
        {
            glTranslatef(0, .5f, 0);
            glScalef(.5f, .5f, 1.f);
            renderDisk(32);
        } 
        glPopMatrix();

        renderSquare(true);
    }
    glPopMatrix();
}
void Ship_render(const Ship *s) {
    Ship_renderBoundingVolumes(s);
}
