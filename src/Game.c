#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <SDL/SDL.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <Utils.h>
#include <Game.h>

void Game_init(Game *g) {
    memset(g, 0, sizeof(*g));
    g->window_size.x = 800;
    g->window_size.y = 600;
    g->bits_per_pixel = 32;
    g->update = Game_updateStartScreen;
    memset(g->views, 0, 2*sizeof(View));
    g->views[0].zoom = 1.f;
    g->views[1].zoom = 1.f;
    g->view_count = 1;
    memset(g->input.players_old, 0, 2*sizeof(PlayerInputState));
    memset(g->input.players, 0, 2*sizeof(PlayerInputState));
}
void Game_deinit(Game *g) {
    /* Rien pour l'instant. */
}

static void Game_quit(Game *g) {
    if(g->fullscreen) {
        g->fullscreen = false;
        Game_reshape(g);
    }
    g->quit = true;
}

void Game_handleEvent(Game *g, const SDL_Event *e) {
    switch(e->type) {
    case SDL_QUIT:
        Game_quit(g);
        break;
    case SDL_VIDEORESIZE:
        g->window_size.x = e->resize.w;
        g->window_size.y = e->resize.h;
        Game_reshape(g);
        break;
    case SDL_KEYDOWN:
        switch(e->key.keysym.sym) {
        case SDLK_SPACE:  g->input.players[0].accelerating  = true; break;
        case SDLK_UP:     g->input.players[0].zooming_in    = true; break;
        case SDLK_DOWN:   g->input.players[0].zooming_out   = true; break;
        case SDLK_LEFT:   g->input.players[0].left_tilting  = true; break;
        case SDLK_RIGHT:  g->input.players[0].right_tilting = true; break;
        case SDLK_LSHIFT: g->input.players[1].accelerating  = true; break;
        case SDLK_z:      g->input.players[1].zooming_in    = true; break;
        case SDLK_s:      g->input.players[1].zooming_out   = true; break;
        case SDLK_q:      g->input.players[1].left_tilting  = true; break;
        case SDLK_d:      g->input.players[1].right_tilting = true; break;
        }
        break;
    case SDL_KEYUP:
        switch(e->key.keysym.sym) {
        case SDLK_SPACE:  g->input.players[0].accelerating  = false; break;
        case SDLK_UP:     g->input.players[0].zooming_in    = false; break;
        case SDLK_DOWN:   g->input.players[0].zooming_out   = false; break;
        case SDLK_LEFT:   g->input.players[0].left_tilting  = false; break;
        case SDLK_RIGHT:  g->input.players[0].right_tilting = false; break;
        case SDLK_LSHIFT: g->input.players[1].accelerating  = false; break;
        case SDLK_z:      g->input.players[1].zooming_in    = false; break;
        case SDLK_s:      g->input.players[1].zooming_out   = false; break;
        case SDLK_q:      g->input.players[1].left_tilting  = false; break;
        case SDLK_d:      g->input.players[1].right_tilting = false; break;

        case SDLK_ESCAPE: 
            Game_quit(g);
            break;
        case SDLK_F11: 
            g->fullscreen = !g->fullscreen;
            Game_reshape(g);
            break;
        case SDLK_F12: Game_takeScreenshot(g); break;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        g->input.mouse_down = true;
        g->input.old_mouse_pos.x = e->button.x;
        g->input.old_mouse_pos.y = e->button.y;
        break;
    case SDL_MOUSEBUTTONUP:
        g->input.mouse_down = false;
        g->input.old_mouse_pos.x = e->button.x;
        g->input.old_mouse_pos.y = e->button.y;
        break;
    case SDL_MOUSEMOTION:
        g->input.old_mouse_pos.x = e->motion.x;
        g->input.old_mouse_pos.y = e->motion.y;
        break;
    }
}

