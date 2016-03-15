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

static void Game_updateMainMenu(Game *g); /* prototype pour Race(). */
static void Game_updateRace(Game *g) { 
    Ship *s = g->ships;
    if(g->input_state.p1.accelerating) {
        const float theta = s->tilt+M_PI/2.f;
        s->accel.x = s->accel_multiplier*cosf(theta);
        s->accel.y = s->accel_multiplier*sinf(theta);
    } else memset(&s->accel, 0, sizeof(Vec2));

    if(g->input_state.p1.left_tilting)
        s->tilt += s->tilt_multiplier;
    if(g->input_state.p1.right_tilting)
        s->tilt -= s->tilt_multiplier;

    rad_modulate(s->tilt);

    s->vel.x += s->accel.x;
    s->vel.y += s->accel.y;
    s->vel.x *= s->friction;
    s->vel.y *= s->friction;
    s->pos.x += s->vel.x;
    s->pos.y += s->vel.y;
    printf("pos:(%f, %f) tilt: %f deg\n", 
            s->pos.x, s->pos.y, degf(s->tilt));
    g->views[0].center.x = s->pos.x;
    g->views[0].center.y = s->pos.y;
    g->views[0].tilt = s->tilt;
    if(g->input_state.p1.zooming_in)
        g->views[0].zoom *= 1.1f;
    if(g->input_state.p1.zooming_out)
        g->views[0].zoom *= .9f;
}
static void Game_updateMapSelection(Game *g) {
    g->update = Game_updateRace;
    g->map.size.x = 20.f;
    g->map.size.y = 20.f;
    memset(g->ships, 0, sizeof(g->ships[0]));
    g->ships[0].accel_multiplier = 0.01f;
    g->ships[0].tilt_multiplier = M_PI/45.f;
    g->ships[0].friction = 0.996f;
    g->update(g);
}
static void Game_updateShipSelection(Game *g) {
    g->update = Game_updateMapSelection;
    g->update(g);
}
static void Game_updateMainMenu(Game *g) { 
    g->update = Game_updateShipSelection;
    g->ship_count = 1;
    g->update(g);
}
void Game_update(Game *g) {
    g->update(g);
}


void Game_init(Game *g) {
    memset(g, 0, sizeof(*g));
    g->window_size.x = 800;
    g->window_size.y = 600;
    g->bits_per_pixel = 32;
    g->view_count = 1;
    g->update = Game_updateMainMenu;
    memset(g->views, 0, sizeof(g->views[0]));
    g->views[0].zoom = 1.f;
}
void Game_deinit(Game *g) {
    /* Rien pour l'instant. */
}

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
            const uint32_t hh = g->window_size.y/2;
            g->views[0].viewport_size.x = hw;
            g->views[0].viewport_size.y = hh;
            g->views[1].viewport_pos.x  = hw;
            g->views[1].viewport_pos.y  = hh;
            g->views[1].viewport_size.x = hw;
            g->views[1].viewport_size.y = hh;
        }
        break;
    }
}
void Game_reshape(Game *g) {
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

static void Game_takeScreenshot(const Game *g) {
    uint8_t *img = malloc(4 * g->window_size.x * g->window_size.y);
    if(!img) {
        fputs("Could not take screenshot: malloc() failed.\n", stderr);
        return;
    }
    glReadPixels(0, 0, g->window_size.x, g->window_size.y, 
                 GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, img);
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
            img, g->window_size.x, g->window_size.y, 32, g->window_size.x*4,
            0xff000000, 0xff0000, 0xff00, 0xff);
    if(!surface) {
        fputs("Could not take screenshot: "
              "SDL_CreateRGBSurfaceFrom() failed.\n",
              stderr);
        free(img);
        return;
    }
    char filename[64];
    snprintf(filename, sizeof(filename), "data/screenshots/%08x-%08x.bmp",
             (uint32_t)time(NULL), SDL_GetTicks());
    int saved = SDL_SaveBMP(surface, filename);
    SDL_FreeSurface(surface);
    free(img);
    if(saved == -1)
        fputs("Could not save screenshot: SDL_SaveBMP() failed.\n", stderr);
    else printf("Saved screenshot to %s\n", filename);
}

void Game_handleEvent(Game *g, const SDL_Event *e) {
    switch(e->type) {
    case SDL_QUIT:
        g->quit = true;
        break;
    case SDL_VIDEORESIZE:
        g->window_size.x = e->resize.w;
        g->window_size.y = e->resize.h;
        Game_reshape(g);
        break;
    case SDL_KEYDOWN:
        switch(e->key.keysym.sym) {
        case SDLK_SPACE: g->input_state.p1.accelerating  = true; break;
        case SDLK_UP:    g->input_state.p1.zooming_in    = true; break;
        case SDLK_DOWN:  g->input_state.p1.zooming_out   = true; break;
        case SDLK_LEFT:  g->input_state.p1.left_tilting  = true; break;
        case SDLK_RIGHT: g->input_state.p1.right_tilting = true; break;
        }
        break;
    case SDL_KEYUP:
        switch(e->key.keysym.sym) {
        case SDLK_SPACE: g->input_state.p1.accelerating  = false; break;
        case SDLK_UP:    g->input_state.p1.zooming_in    = false; break;
        case SDLK_DOWN:  g->input_state.p1.zooming_out   = false; break;
        case SDLK_LEFT:  g->input_state.p1.left_tilting  = false; break;
        case SDLK_RIGHT: g->input_state.p1.right_tilting = false; break;

        case SDLK_ESCAPE: g->quit = true; break;
        case SDLK_F11: 
            g->fullscreen = !g->fullscreen;
            Game_reshape(g);
            break;
        case SDLK_F12: Game_takeScreenshot(g); break;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        g->input_state.mouse_down = true;
        g->input_state.old_mouse_pos.x = e->button.x;
        g->input_state.old_mouse_pos.y = e->button.y;
        break;
    case SDL_MOUSEBUTTONUP:
        g->input_state.mouse_down = false;
        g->input_state.old_mouse_pos.x = e->button.x;
        g->input_state.old_mouse_pos.y = e->button.y;
        break;
    case SDL_MOUSEMOTION:
        g->input_state.old_mouse_pos.x = e->motion.x;
        g->input_state.old_mouse_pos.y = e->motion.y;
        break;
    }
}

static void Game_renderView(const Game *g, const View *v) {
    /*
     * La matrice de projection N'EST PAS pour la caméra. C'est le rôle
     * de MODELVIEW à la place.
     */

    glViewport(v->viewport_pos.x, v->viewport_pos.y,
               v->viewport_size.x, v->viewport_size.y);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /* FIXME Sûrement bogus si la map est plus haute que large. */
    const float w = max(g->map.size.x, g->map.size.y);
    gluOrtho2D(-w, w, 
        -w*v->viewport_size.y/(float)v->viewport_size.x, 
         w*v->viewport_size.y/(float)v->viewport_size.x);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    View_apply(v);

    Map_render(&g->map);
    size_t i;
    for(i=0 ; i<g->ship_count ; ++i)
        Ship_render(g->ships+i);
}

void Game_render(const Game *g) {
    size_t i;
    for(i=0 ; i<g->view_count ; ++i)
        Game_renderView(g, g->views+i);
}


