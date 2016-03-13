#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <SDL/SDL.h>

typedef struct {
    float left, right, bottom, top;
} Ortho2d;

typedef struct {
    unsigned quit : 1;
    uint16_t win_w, win_h;
    uint32_t bits_per_pixel;
    Ortho2d ortho2d;
} Game;

void Game_init(Game *g);
void Game_deinit(Game *g);
/* Redimensionne la fen^etre conformément aux champs win_w, win_h et ortho2d. */
void Game_reshape(Game *g);
void Game_handleEvent(Game *g, const SDL_Event *e);
/* Mise à jour de la logique du jeu. Découplée de la gestion d'évènements. */
void Game_update(Game *g);
/* Affichage du jeu. Que des draw calls : pas de glClear() ni de SwapBuffers(). */
void Game_render(const Game *g);



/* TODO Ces définitions méritent peut-^etre d'^etre dans un autre fichier. */
typedef float vec2[2];
typedef uint32_t uvec2[2];

static inline void windowToWorld(vec2 out, const uvec2 coord, const uvec2 win_size, const Ortho2d *ortho) {
    out[0] = ortho->left + (ortho->right - ortho->left)*coord[0]/(float)win_size[0];
    out[1] = ortho->top  - (ortho->top - ortho->bottom)*coord[1]/(float)win_size[1];
}
/* TODO Test me ! */
static inline void worldToWindow(uvec2 out, const vec2 coord, const uvec2 win_size, const Ortho2d *ortho) {
    out[0] = win_size[0]*(coord[0]-ortho->left)/(ortho->right - ortho->left);
    // Celle-là est mauvaise. FIXME
    //out[1] = -win_size[1]*(coord[1]-ortho->bottom)/(ortho->top - ortho->bottom);
}

#endif /* GAME_H */
