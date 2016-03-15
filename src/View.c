#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <Utils.h>
#include <View.h>

void View_apply(const View *v) {
    glScalef(v->zoom, v->zoom, 1);
    glRotatef(degf(-v->tilt), 0,0,1);
    glTranslatef(-v->center.x, -v->center.y, 0);
}

/* Ces deux fonctions pourraient mettre les résultat de leurs calculs dans 
 * un cache.
 * Je ne le fais pas parce que je suppose que la View change en permanence. */
void View_mapPixeltoCoords(const View *v, Vec2 *coords, const Vec2u *pixel) {
    GLdouble modelview[16], proj[16];
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    {
        glLoadIdentity();
        View_apply(v);
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    }
    glPopMatrix();
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    GLint viewport[4] = {
        v->viewport_pos.x, v->viewport_pos.y, 
        v->viewport_size.x, v->viewport_size.y
    };
    GLdouble cx, cy, cz;
    gluUnProject(pixel->x, pixel->y, 0, 
                 modelview, proj, viewport, 
                 &cx, &cy, &cz);
    coords->x = cx;
    coords->y = cy;
}

void View_mapCoordstoPixel(const View *v, Vec2u *pixel, const Vec2 *coords) {
    GLdouble modelview[16], proj[16];
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    {
        glLoadIdentity();
        View_apply(v);
        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    }
    glPopMatrix();
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    GLint viewport[4] = {
        v->viewport_pos.x, v->viewport_pos.y, 
        v->viewport_size.x, v->viewport_size.y
    };
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
