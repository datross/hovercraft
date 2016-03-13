#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <Game.h>

void Game_init(Game *g) {
    /* TODO */
}
void Game_deinit(Game *g) {
    /* TODO */
}
void Game_resizeWindow(const Game *g) {
    if(!SDL_SetVideoMode(g->win_w, g->win_h, g->bits_per_pixel, 
                SDL_OPENGL | SDL_DOUBLEBUF | SDL_RESIZABLE)) {
        fprintf(stderr, "Impossible d'ouvrir la fenetre. Fin du programme.\n");
        /* TODO afficher une erreur plus détaillée ? */
        exit(EXIT_FAILURE);
    }
    glViewport(0, 0, g->win_w, g->win_h);
}

void Game_resizeWorld(const Game *g) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#define o (g->ortho2d)
    gluOrtho2D(o.left, o.right, o.bottom, o.top);
#undef o
}

void Game_handleEvent(Game *g, const SDL_Event *e) {
    switch(e->type) {
        case SDL_QUIT:
            g->quit = true;
            break;
        case SDL_VIDEORESIZE:
            g->win_w = e->resize.w;
            g->win_h = e->resize.h;
            Game_reshape(g);
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


