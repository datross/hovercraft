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
void View_mapPixeltoCoords(const View *v, Vec2 *coords, const Vec2u *pixel);
void View_mapCoordstoPixel(const View *v, Vec2u *pixel, const Vec2 *coords);

#endif /* VIEW_H */
