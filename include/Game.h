#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <SDL/SDL.h>

#include <Vec.h>
#include <View.h>
#include <Map.h>
#include <Ship.h>

typedef struct {
    unsigned accelerating  : 1;
    unsigned left_tilting  : 1;
    unsigned right_tilting : 1;
    unsigned zooming_in    : 1;
    unsigned zooming_out   : 1;
    unsigned reserved      : 3;
} PlayerInputState;

typedef struct {
    PlayerInputState players[2];
    Vec2u old_mouse_pos;
    unsigned mouse_down : 1;
    unsigned reserved   : 7;
} InputState;

typedef struct Game Game; /* Déclaration pour le pointeur de fonction */
struct Game {
    uint32_t tickrate;       /* public. Spécifie le taux de rafraichissement
                                de la logique du jeu en millisecondes
                                (aka nombre de Game_update()s par millisecondes)
                                Dans notre cas c'est
                                la framerate (voir 'main.c'), mais Game
                                n'est pas censé le savoir. 
                                'tickrate' est la référence pour implémenter le
                                countdown ou un compteur de vitesse. */
    uint32_t bits_per_pixel; /* public. */
    Vec2u old_window_size;   /* Pour sauvegarder la taille de la fenetre avant 
                                d'entrer en fullscreen. */
    Vec2u window_size;       /* public. Appeler Game_reshape() en cas de 
                                changement. */
    InputState input;  /* privé. Altéré par Game_handleEvent(). */
    void (*update)(Game*);   /* privé. Appeler Game_update() à la place. */
    Map map;
    Ship ships[2];
    View views[2]; /* Nombre magique + taille fixe, mais pour le coup 
                      un malloc() est trop chiant. 
                      On va voir avec le temps. */
    unsigned view_count : 2; /* Idem, taille fixe. */
    unsigned ship_count : 2;
    unsigned quit       : 1;
    unsigned fullscreen : 1;
    unsigned reserved   : 2; /* Padding pour que le bitfield fasse un octet. */
    uint32_t race_step_ms;
     int32_t race_time_ms;
};

/* Game_init() seul ne suffit pas à afficher le jeu. 
 * Appeler Game_reshape() pour ça. */
void Game_init(Game *g);
void Game_deinit(Game *g);
void Game_reshape(Game *g);
void Game_handleEvent(Game *g, const SDL_Event *e);
/* Mise à jour de la logique du jeu. Découplée de la gestion d'évènements. */
void Game_update(Game *g);
/* Affichage du jeu. 
 * Que des draw calls : pas de glClear() ni de SwapBuffers(). */
void Game_render(const Game *g);


#endif /* GAME_H */
