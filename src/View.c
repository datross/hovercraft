#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <Utils.h>
#include <View.h>

void View_apply(const View *v) {
    glScalef(v->zoom, v->zoom, 1);
    glRotatef(degf(-v->tilt), 0,0,1);
    glTranslatef(-v->center.x, -v->center.y, 0);
/*
 * On devrait pouvoir conserver le résultat comme ça :
 *    GLdouble mat[16];
 *    glGetDoublev(GL_MODELVIEW_MATRIX, mat);
 * Mais sûrement overkill si la vue change à chaque frame.
 * L'avantage c'est qu'on peut s'en servir pour gluProject().
 */
}
