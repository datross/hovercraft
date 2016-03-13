#ifndef VIEW_H
#define VIEW_H

#include "Vec.h"

typedef struct {
    float left, right, bottom, top;
} View;

static inline void View_mapPixelToCoords(Vec2 *out, const Vec2u *coord, 
                           const Vec2u *win_size, const View *view) {
    out->x = view->left + (view->right - view->left)*coord->x/(float)win_size->x;
    out->y = view->top  - (view->top - view->bottom)*coord->y/(float)win_size->y;
}
/* TODO Test me ! */
static inline void View_mapCoordToPixels(Vec2u *out, const Vec2u *coord, 
                          const Vec2u *win_size, const View *view) {
    out->x = win_size->x*(coord->x-view->left)/(view->right - view->left);
    // Celle-là est bogus. FIXME
    //out->y = -win_size->y*(coord->y-view->bottom)/(view->top - view->bottom);
}

#endif /* VIEW_H */
