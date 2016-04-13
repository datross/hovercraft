#ifndef VIEW_H
#define VIEW_H

#include <string.h>
#include <Vec.h>
#include <Ship.h>

#define VIEW_FOLLOW_SPEED
#define VIEW_

typedef struct {
    Vec2u viewport_pos, viewport_size; /* Window coordinates */
    Vec2 center; /* World coordinates */
    float zoom, tilt; /* tilt = inclinaison, en radians. */
    float ortho_right; /* pour gluOrtho2D(). 
                         S'il vaut 1 par exemple, notre monde va de -1 à 1
                         horizontalement, et l'unité verticale est choisie
                         de sorte à préserver l'aspect ratio. 
                         Pour les menus on le voudra à 1. In-game, on le
                         choisit avant la course par rapport à la taille des 
                         véhicules (Typiquement 8). */
} View;

void View_apply(const View *v);
void View_mapPixelToCoords(const View *v, Vec2 *coords, const Vec2u *pixel);
/* Fonction non-implémentée. */
/* void View_mapCoordsToPixel(const View *v, Vec2u *pixel, const Vec2 *coords); */

#endif /* VIEW_H */
