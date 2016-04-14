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

static void Game_resizeViewports(Game *g) {
    assert(g->view_count <= 2);
    switch(g->view_count) {
    case 1: 
        g->views[0].viewport_pos.x  = 0;
        g->views[0].viewport_pos.y  = 0;
        g->views[0].viewport_size.x = g->window_size.x;
        g->views[0].viewport_size.y = g->window_size.y;
        break;
    case 2: 
        g->views[0].viewport_pos.x  = 0;
        g->views[0].viewport_pos.y  = 0;
        {
            const uint32_t hw = g->window_size.x/2;
            g->views[0].viewport_size.x = hw;
            g->views[0].viewport_size.y = g->window_size.y;
            g->views[1].viewport_pos.x  = hw;
            g->views[1].viewport_pos.y  = 0;
            g->views[1].viewport_size.x = hw;
            g->views[1].viewport_size.y = g->window_size.y;
        }
        break;
    }
}
void Game_reshape(Game *g) {
    glEnable(GL_TEXTURE_2D);
    if(g->fullscreen) {
        /* La doc ne dit pas s'il faut libérer le tableau à la fin. */
        SDL_Rect **modes = SDL_ListModes(NULL, SDL_FULLSCREEN);
        switch((intptr_t)modes) {
        case  0: /* Aucun mode n'est disponible. */ break;
        case -1: /* Tout mode peut convenir. */ break;
        /* Dans les deux cas ci-dessus, on peut toujours 
         * passer en fullscreen, mais c'est juste moche. */
        default: /* On a une liste de modes. */
            g->old_window_size.x = g->window_size.x;
            g->old_window_size.y = g->window_size.y;
            g->window_size.x = modes[0]->w;
            g->window_size.y = modes[0]->h;
            break;
        }
    } else if(g->old_window_size.x) {
        g->window_size.x = g->old_window_size.x;
        g->window_size.y = g->old_window_size.y; 
        g->old_window_size.x = 0;
    }
    if(!SDL_SetVideoMode(
            g->window_size.x, 
            g->window_size.y, 
            g->bits_per_pixel, 
            SDL_OPENGL | SDL_DOUBLEBUF | SDL_RESIZABLE
            |(g->fullscreen ? SDL_FULLSCREEN : 0))) {
        fprintf(stderr, "SDL_SetVideoMode() failed :\n\t%s\n Exitting.\n",
                        SDL_GetError());
        exit(EXIT_FAILURE);
    }
    Game_resizeViewports(g);
}

void Game_takeScreenshot(const Game *g) {
    uint8_t *lin = malloc(4 * g->window_size.x);
    if(!lin) {
        fputs("Could not take screenshot: malloc() failed.\n", stderr);
        return;
    }
    uint8_t *img = malloc(4 * g->window_size.x * g->window_size.y);
    if(!img) {
        fputs("Could not take screenshot: malloc() failed.\n", stderr);
        free(lin);
        return;
    }
    glReadPixels(0, 0, g->window_size.x, g->window_size.y, 
                 GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, img);
    /* Faire la symétrie verticale.*/
    size_t linesize = 4*g->window_size.x;
    size_t t, b; /* top, bottom */
    for(t=0, b=g->window_size.y-1 ; t<b ; ++t, --b) {
        memcpy(lin,            img+t*linesize, linesize);
        memcpy(img+t*linesize, img+b*linesize, linesize);
        memcpy(img+b*linesize, lin,            linesize);
    }
    free(lin);
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
            img, g->window_size.x, g->window_size.y, 32, linesize,
            0xff000000, 0xff0000, 0xff00, 0xff);
    if(!surface) {
        fputs("Could not take screenshot: "
              "SDL_CreateRGBSurfaceFrom() failed.\n",
              stderr);
        free(img);
        return;
    }
    char filename[64];
    snprintf(filename, sizeof(filename), "data/screenshots/%010u-%010u.bmp",
             (uint32_t)time(NULL), SDL_GetTicks());
    int saved = SDL_SaveBMP(surface, filename);
    SDL_FreeSurface(surface);
    free(img);
    if(saved == -1)
        fputs("Could not save screenshot: SDL_SaveBMP() failed.\n", stderr);
    else printf("Saved screenshot to %s\n", filename);
}

static void Game_renderWorldMouseCursor(const Game *g) {
    glColor3f(1,1,1);
    glPointSize(4.f);
    glBegin(GL_POINTS);
    glVertex2f(g->world_mouse_cursor.x, g->world_mouse_cursor.y);
    glEnd();
}

static void Game_renderView(const Game *g, size_t view_index) {
    const View *v = g->views + view_index;

    glViewport(v->viewport_pos.x, v->viewport_pos.y,
               v->viewport_size.x, v->viewport_size.y);

    View_apply(v);

    Map_render(&g->map);
    size_t i;
    for(i=0 ; i<g->ship_count ; ++i) {
        Ship_render(g->ships+i);
        if(i==view_index) {
            Map_renderCheckpoints(&g->map, g->ships[i].next_checkpoint_index);
            Ship_renderGuides(g->ships+i);
        }
    }
    Game_renderWorldMouseCursor(g);
}

void Game_render(const Game *g) {
    size_t i;
    for(i=0 ; i<g->view_count ; ++i)
        Game_renderView(g, i);
}

