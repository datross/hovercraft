#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <Utils.h>
#include <View.h>


static inline void View_applyProj(const View *v) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#define w (v->ortho_right)
    gluOrtho2D(-w, w, 
        -w*v->viewport_size.y/(float)v->viewport_size.x, 
         w*v->viewport_size.y/(float)v->viewport_size.x);
#undef w
}
static inline void View_applyModelView(const View *v) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(v->zoom, v->zoom, 1);
    glRotatef(degf(-v->tilt+M_PI/2.f), 0,0,1);
    glTranslatef(-v->center.x, -v->center.y, 0);
}
void View_apply(const View *v) {
    View_applyProj(v);
    View_applyModelView(v);
}

/* Matrice ModelView honnêtement bricolée seulement pour que
 * View_mapPixelToCoords() marche. */
static inline void View_applyTweakedModelView(const View *v) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(v->zoom, v->zoom, 1);
    glRotatef(degf(v->tilt-M_PI/2.f), 0,0,1);
    glTranslatef(-v->center.x, v->center.y, 0);
}
/* Bout de Code commun à View_mapPixelToCoords() et View_mapCoordsToPixel(). */
static void View_prepareMapping(const View *v, GLdouble modelview[16], 
                                GLdouble proj[16], GLint viewport[4]) {

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    {
        View_applyProj(v);
        glGetDoublev(GL_PROJECTION_MATRIX, proj);
    }
    glPopMatrix();
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    {
        View_applyTweakedModelView(v);
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    }
    glPopMatrix();
    viewport[0] = v->viewport_pos.x;
    viewport[1] = v->viewport_pos.y;
    viewport[2] = v->viewport_size.x;
    viewport[3] = v->viewport_size.y;
}

void View_mapPixelToCoords(const View *v, Vec2 *coords, const Vec2u *pixel) {
    GLdouble modelview[16], proj[16];
    GLint viewport[4];
    View_prepareMapping(v, modelview, proj, viewport);
    GLdouble cx, cy, cz;
    gluUnProject(pixel->x, pixel->y, 0, 
                 modelview, proj, viewport, 
                 &cx, &cy, &cz);
    coords->x = cx;
    coords->y = -cy;
}

/* FIXME cette fonction ne marche pas. 
 * Je l'abandonne car en fin de compte je ne vois pas de cas d'utilisation
 * concrêt. */
/*
void View_mapCoordsToPixel(const View *v, Vec2u *pixel, const Vec2 *coords) {
    GLdouble modelview[16], proj[16];
    GLint viewport[4];
    View_prepareMapping(v, modelview, proj, viewport);
    GLdouble px, py, pz;
    gluProject(coords->x, coords->y, 0, 
               modelview, proj, viewport, 
               &px, &py, &pz);
    pixel->x = px;
    pixel->y = v->viewport_size.y - py;
}
*/

