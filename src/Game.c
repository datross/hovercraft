#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <Game.h>

void Game_init(Game *g) {
    g->window_size.x = 800;
    g->window_size.y = 600;
    g->bits_per_pixel = 32;
    g->view.left   = -1.f;
    g->view.right  =  1.f;
    g->view.bottom = -1.f;
    g->view.top    =  1.f;
}
void Game_deinit(Game *g) {
    /* TODO */
}
void Game_resizeWindow(const Game *g) {
    if(!SDL_SetVideoMode(g->window_size.x, g->window_size.y, g->bits_per_pixel, 
                SDL_OPENGL | SDL_DOUBLEBUF | SDL_RESIZABLE)) {
        fprintf(stderr, "Impossible d'ouvrir la fenetre. Fin du programme.\n");
        /* TODO afficher une erreur plus détaillée ? */
        exit(EXIT_FAILURE);
    }
    glViewport(0, 0, g->window_size.x, g->window_size.y);
}

void Game_resizeView(const Game *g) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#define o (g->view)
    gluOrtho2D(o.left, o.right, o.bottom, o.top);
#undef o
}

void Game_handleEvent(Game *g, const SDL_Event *e) {
    switch(e->type) {
        case SDL_QUIT:
            g->quit = true;
            break;
        case SDL_VIDEORESIZE:
            g->window_size.x = e->resize.w;
            g->window_size.y = e->resize.h;
            Game_resizeWindow(g);
            break;
        default:
            break;
    }
}

void Game_update(Game *g) {
    /* TODO */
}
void Game_render(const Game *g) {
    /* TODO */
}


