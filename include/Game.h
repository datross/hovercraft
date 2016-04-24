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
#include <GameLimits.h>

typedef struct {
    unsigned accelerating  : 1;
    unsigned left_tilting  : 1;
    unsigned right_tilting : 1;
    unsigned zooming_in    : 1;
    unsigned zooming_out   : 1;
    unsigned escaping      : 1;
    unsigned reserved      : 2;
} PlayerInputState;

typedef struct {
    PlayerInputState now, old;
} PlayerInput;

#define PLAYERINPUT_PRESSED(ipt, cmd) (ipt.now.cmd > ipt.old.cmd)

typedef struct {
    PlayerInput players[MAX_PLAYERS];
    Vec2u old_mouse_pos;
    unsigned mouse_down : 1;
    unsigned reserved   : 7;
} InputState;

typedef struct {
    Sprite sky, bg[4], title, oneplayer, twoplayers;
    unsigned choice : 1;
    unsigned reserved : 7;
} MainMenu;

typedef struct {
    Sprite titles[MAX_PLAYERS], ship_cell;
    size_t selected_ship_index[MAX_PLAYERS];
    size_t selected_pal_index[MAX_PLAYERS];
    unsigned player_index : 1;
    unsigned reserved : 7;
} ShipMenu;

typedef struct {
    Sprite left_arrow;
    size_t selected_map_index;
} MapMenu;

typedef struct ClapTransition ClapTransition;
struct ClapTransition {
    Sprite top, bottom;
    Vec2 top_pos, bottom_pos;
    uint32_t time_of_closing;
    void (*update)(ClapTransition *ct, float top);
};

typedef struct {
    uint32_t step_ms;
     int32_t time_ms;
    unsigned countdown_prev_seconds_left;
    Map  map;
    Ship ships[MAX_PLAYERS];
    View views[MAX_PLAYERS];
    PhysicWorld world;
    unsigned ship_count : 2;
    unsigned view_count : 2;
    unsigned reserved   : 4;
    uint32_t completion_times[MAX_PLAYERS];
    uint32_t time_of_completion;
    uint32_t rankings[MAX_PLAYERS];
} Race;

typedef struct Game Game; /* Déclaration pour le pointeur de fonction */
#define FADE_TRANSITION_INC .08f
typedef struct {
    float alpha, alpha_vel;
    void (*next_render_func)(const Game*);
    void (*update)(Game*); /* Mettre à jour la transition. */
} FadeTransition;

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
    void (*render)(const Game*);   /* privé. Appeler Game_render() à la place. */
    unsigned quit         : 1;
    unsigned fullscreen   : 1;
    unsigned player_count : 2;
    unsigned reserved     : 4; /* Padding pour que le bitfield fasse un octet. */
    View menu_view;
    Vec2 world_mouse_cursor;
    MainMenu main_menu;
    ShipMenu ship_menu;
    MapMenu map_menu;
    FadeTransition fade_transition;
    ClapTransition clap_transition;
    Race race;
    ShipData ship_data[MAX_SHIPS];
    MapData map_data[MAX_MAPS];
    size_t map_data_count;
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
/* Celle-là pour Game_graphics.c */
void Game_updatePaletteSelection(Game *g);

void ClapTransition_updateDummy(ClapTransition *ct, float top);
void FadeTransition_updateDummy(Game *g);

/* Ces fonctions sont privées. Elles sont utilisées en tant que pointeurs
 * par Game_logic.c */
void Game_renderMainMenu(const Game *g);
void Game_renderShipMenu(const Game *g);
void Game_renderMapMenu(const Game *g);
void Game_renderMapMenuWithClap(const Game *g);
void Game_renderRace(const Game *g);
void Game_renderRaceWithClap(const Game *g);
void Game_renderStartScreen(const Game *g);

/* Encore des fonctions privées. */
void Game_loadMenus(Game *g, const char *dirname);
void Game_loadShips(Game *g, const char *dirname);
void Game_loadMaps(Game *g, const char *dirname);

void Game_resizeViewports(Game *g);

#endif /* GAME_H */
