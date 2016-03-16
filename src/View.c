#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <Utils.h>
#include <View.h>

void View_apply(const View *v) {

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#define w (v->ortho_right)
    gluOrtho2D(-w, w, 
        -w*v->viewport_size.y/(float)v->viewport_size.x, 
         w*v->viewport_size.y/(float)v->viewport_size.x);
#undef w

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(v->zoom, v->zoom, 1);
    glRotatef(degf(-v->tilt), 0,0,1);
    glTranslatef(-v->center.x, -v->center.y, 0);
}

/* Bout de Code commun à View_mapPixeltoCoords() et View_mapCoordstoPixel(). */
static void View_prepareMapping(const View *v, GLdouble modelview[16], 
                                GLdouble proj[16], GLint viewport[4]) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    View_apply(v);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    viewport[0] = v->viewport_pos.x;
    viewport[1] = v->viewport_pos.y;
    viewport[2] = v->viewport_size.x;
    viewport[3] = v->viewport_size.y;
}

void View_mapPixeltoCoords(const View *v, Vec2 *coords, const Vec2u *pixel) {
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

void View_mapCoordstoPixel(const View *v, Vec2u *pixel, const Vec2 *coords) {
    GLdouble modelview[16], proj[16];
    GLint viewport[4];
    View_prepareMapping(v, modelview, proj, viewport);
    GLdouble px, py, pz;
    gluProject(coords->x, coords->y, 0, 
               modelview, proj, viewport, 
               &px, &py, &pz);
    pixel->x = px;
    pixel->y = py;
}

/* Ancien code, au cas où : */
/*
static inline void View_mapPixelToCoords(Vec2 *out, const Vec2u *coord, 
                           const Vec2u *win_size, const View *view) {
    out->x = view->left + (view->right - view->left)*coord->x/(float)win_size->x;
    out->y = view->top  - (view->top - view->bottom)*coord->y/(float)win_size->y;
}
static inline void View_mapCoordToPixels(Vec2u *out, const Vec2u *coord, 
                          const Vec2u *win_size, const View *view) {
    out->x = win_size->x*(coord->x-view->left)/(view->right - view->left);
    out->y = win_size->y*(1.f - (coord->y + view->bottom)/(view->top - view->bottom));
}
*/
