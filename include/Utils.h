#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <math.h>
#include <GL/gl.h>

#define max(a,b) ((a)>(b) ? (a) : (b))
#define degf(a) ((a)*180.f/M_PI)
#define radf(a) ((a)*M_PI/180.f)

static inline void renderSquare(bool full) {
    glBegin(full ? GL_QUADS : GL_LINE_LOOP);
    glVertex2f(-.5, -.5);
    glVertex2f( .5, -.5);
    glVertex2f( .5,  .5);
    glVertex2f(-.5,  .5);
    glEnd();
}

static inline void renderIsocelesTriangle(bool full) {
    glBegin(full ? GL_TRIANGLES : GL_LINE_LOOP);
    glVertex2f(  0,  .5);
    glVertex2f(-.5, -.5);
    glVertex2f( .5, -.5);
    glEnd();
}

static inline void renderAxes(void) {
    GLfloat col[4];
    glGetFloatv(GL_CURRENT_COLOR, col);

    glColor3f(0,1,0);
    glBegin(GL_LINES);
    glVertex2f(0, 0);
    glVertex2f(0, 1);
    glEnd();

    glColor3f(1,0,0);
    glBegin(GL_LINES);
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glEnd();

    glColor3f(0,0,1);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1);
    glEnd();


    glColor4f(col[0], col[1], col[2], col[3]);
}

#define renderDisk(seg) renderCircle(seg, true)
static inline void renderCircle(unsigned seg, bool full) {
    if(seg<3)
        return;
    glBegin(full ? GL_POLYGON : GL_LINE_LOOP);
    unsigned i;
    for(i=1 ; i<=seg ; ++i) {
        float theta = 2*M_PI*i/(float)seg;
        glVertex2f(cosf(theta), sinf(theta));
    }
    glEnd();
}

#endif /* UTILS_H */
