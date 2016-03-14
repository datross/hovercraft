#ifndef VIEW_H
#define VIEW_H

#include <string.h>
#include <Vec.h>

typedef struct {
    Vec2u viewport_pos, viewport_size; /* Window coordinates */
    Vec2 center; /* World coordinates */
    float zoom, tilt; /* tilt = inclinaison, en radians. */
} View;

void View_apply(const View *v);
/* TODO Il serait mieux d'utiliser gluproject() et gluUnproject() à la place. */

/* Ces fonctions n'ont été testées que sur le papier. */
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

#endif /* VIEW_H */
