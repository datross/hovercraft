#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <SDL/SDL.h>

#include <Vec.h>
#include <View.h>

typedef struct {
    unsigned quit : 1;
    uint32_t bits_per_pixel;
    Vec2u window_size;
    View view;
} Game;

void Game_init(Game *g);
void Game_deinit(Game *g);
/* Redimensionne la fen^etre conformément aux champs win_w, win_h et view. */
void Game_resizeWindow(const Game *g);
void Game_resizeView(const Game *g);
static inline void Game_reshape(const Game *g) {
    Game_resizeWindow(g);
    Game_resizeView(g);
}
void Game_handleEvent(Game *g, const SDL_Event *e);
/* Mise à jour de la logique du jeu. Découplée de la gestion d'évènements. */
void Game_update(Game *g);
/* Affichage du jeu. Que des draw calls : pas de glClear() ni de SwapBuffers(). */
void Game_render(const Game *g);


#endif /* GAME_H */
