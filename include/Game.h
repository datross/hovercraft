#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <SDL/SDL.h>

#include <Vec.h>
#include <View.h>
#include <Map.h>
#include <Ship.h>
#include <Physics.h>
#include <Sprite.h>

#define MAX_SHIPS 4
#define MAX_PALETTES 6
#define MAX_MAPS 32
#define MAX_PLAYERS 2

typedef struct {
    unsigned accelerating  : 1;
    unsigned left_tilting  : 1;
    unsigned right_tilting : 1;
    unsigned zooming_in    : 1;
    unsigned zooming_out   : 1;
    unsigned reserved      : 3;
} PlayerInputState;

typedef struct {
    PlayerInputState now, old;
} PlayerInput;

#define PlayerInput_pressed(ipt, cmd) (ipt.now.cmd < ipt.old.cmd)

typedef struct {
    PlayerInput players[MAX_PLAYERS];
    Vec2u old_mouse_pos;
    unsigned mouse_down : 1;
    unsigned reserved   : 7;
} InputState;

typedef struct {
    Sprite bg, title, p1, p2;
    unsigned choice : 1;
    unsigned reserved : 7;
} MainMenu;

typedef struct {
    Sprite bg;
    Sprite ship_artworks[MAX_SHIPS][MAX_PALETTES];
    Sprite ship_icons[MAX_SHIPS][MAX_PALETTES];
    unsigned player_index : 1;
    unsigned selected_ship_index : 3;
    unsigned reserved : 4;
} ShipMenu;

typedef struct {
    Sprite bg;
    Sprite map_artworks[MAX_MAPS];
    Sprite map_names[MAX_MAPS];
    size_t map_count, selected_map_index;
} MapMenu;

typedef struct {
    Sprite top, bottom;
    Vec2 top_pos, bottom_pos;
} ClapTransition;

typedef struct {
    float opacity;
} FadeTransition;

typedef struct {
    uint32_t step_ms;
     int32_t time_ms;
    Map map;
    Ship ships[MAX_PLAYERS];
    View views[MAX_PLAYERS]; /* Nombre magique + taille fixe, mais pour le coup 
                      un malloc() est trop chiant. 
                      On va voir avec le temps. */
    PhysicWorld world;
    unsigned ship_count : 2;
    unsigned view_count : 2; /* Idem, taille fixe. */
    unsigned reserved   : 4;
} Race;

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
    unsigned quit       : 1;
    unsigned fullscreen : 1;
    unsigned reserved   : 6; /* Padding pour que le bitfield fasse un octet. */
    View menu_view;
    Vec2 world_mouse_cursor;
    MainMenu main_menu;
    ShipMenu ship_menu;
    MapMenu map_menu;
    ClapTransition clap_transition;
    FadeTransition fade_transition;
    Race race;
};

/* Game_init() seul ne suffit pas à afficher le jeu. 
 * Appeler Game_reshape() pour ça. */
void Game_init(Game *g);
void Game_deinit(Game *g);
void Game_reshape(Game *g);
void Game_handleEvent(Game *g, const SDL_Event *e);
/* Elle est appellée automatiquement par handleEvenet(). */
void Game_takeScreenshot(const Game *g);
/* Mise à jour de la logique du jeu. Découplée de la gestion d'évènements. */
void Game_update(Game *g);
/* Affichage du jeu. 
 * Que des draw calls : pas de glClear() ni de SwapBuffers(). */
void Game_render(const Game *g);


/* Cette fonction est privée. Elle sert juste de passerelle entre 
 * Game.c et Game_logic.c */
void Game_updateStartScreen(Game *g);

#endif /* GAME_H */
