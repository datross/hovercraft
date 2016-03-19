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

void Ship_refreshGuides(Ship *s) {
    size_t i;
    for(i=0 ; i<s->guide_count ; ++i) {
        Vec2 diff = {s->guides[i].pos.x - s->pos.x, 
                     s->guides[i].pos.y - s->pos.y};
        s->guides[i].theta = degf(atan2f(diff.y, diff.x))-90.f;
        s->guides[i].distance = sqrtf(diff.x*diff.x + diff.y*diff.y);
        /*
        printf("Distance from guides[%zu] : %u units.\n", 
                i, (uint32_t)s->guides[i].distance);
        */
    }
}

static void Game_quit(Game *g) {
    if(g->fullscreen) {
        g->fullscreen = false;
        Game_reshape(g);
    }
    g->quit = true;
}

static void Game_updateMainMenu(Game *g); /* prototype pour Race(). */

static void Game_updateRace(Game *g) { 

    uint32_t new_ms = SDL_GetTicks();
    g->race_time_ms += new_ms - g->race_step_ms;
    g->race_step_ms = new_ms;

    size_t i;
    for(i=0 ; i<g->ship_count ; ++i) {
        Ship *s = g->ships+i;

        if(g->input.players[i].accelerating) {
            const float theta = s->tilt+M_PI/2.f;
            s->accel.x = s->accel_multiplier*cosf(theta);
            s->accel.y = s->accel_multiplier*sinf(theta);
        } else memset(&s->accel, 0, sizeof(Vec2));

        if(g->input.players[i].left_tilting)
            s->tilt += s->tilt_step;
        if(g->input.players[i].right_tilting)
            s->tilt -= s->tilt_step;

        s->tilt = fmodf(s->tilt, 2*M_PI);

        /* La friction joue beaucoup sur la vitesse maximale
         * factuellement atteignable des véhicules. */
        s->vel.x *= s->friction * g->map.friction;
        s->vel.y *= s->friction * g->map.friction;
        s->vel.x += s->accel.x;
        s->vel.y += s->accel.y;
        s->pos.x += s->vel.x;
        s->pos.y += s->vel.y;
        const float dist = sqrtf(s->vel.x*s->vel.x + s->vel.y*s->vel.y);
        const float speed = 1000.f*dist/g->tickrate;
        if(speed > s->max_speed) {
            printf("Reaching max speed.\n");
            /* FIXME C'est une approximation qui est précise
             * avec 'max_speed' élevée, mais pas tant que ça
             * quand elle est faible. */
            s->vel.x *= s->max_speed/speed;
            s->vel.y *= s->max_speed/speed;
        }

        {
            const Checkpoint *c = g->map.checkpoints 
                                  + s->next_checkpoint_index;
            Vec2 diff = { c->pos.x - s->pos.x, c->pos.y - s->pos.y };
            float dist = sqrtf(diff.x*diff.x + diff.y*diff.y);
            if(dist <= c->radius)
                ++(s->next_checkpoint_index);
            if(s->next_checkpoint_index >= g->map.checkpoint_count) {
                printf("It's over ! Player %zu won.\n", i+1);
                printf("elapsed time : %d ms\n", g->race_time_ms);
                /* Game_quit(g); */
                s->next_checkpoint_index = 0; /* Bien sûr on retire ça après. */
            } else {
                s->guides[0].pos.x = c->pos.x;
                s->guides[0].pos.y = c->pos.y;
            }
       }

        const size_t j = (i+1)%(g->ship_count);
        s->guides[1].pos.x = g->ships[j].pos.x;
        s->guides[1].pos.y = g->ships[j].pos.y;
        Ship_refreshGuides(s);

        /*
        printf("pos:(%f, %f); tilt: %f deg; speed: %u units/s\n", 
               s->pos.x, s->pos.y, degf(s->tilt), (uint32_t)speed);
        */

        if(i>=g->view_count)
            break;
        g->views[i].center.x = s->pos.x;
        g->views[i].center.y = s->pos.y;
        g->views[i].tilt = s->tilt;
        if(g->input.players[i].zooming_in && g->views[i].zoom < 6.f)
            g->views[i].zoom *= 1.1f;
        if(g->input.players[i].zooming_out && g->views[i].zoom > 0.02f)
            g->views[i].zoom *= .9f;
        /* if(i==0)printf("zoom : %f\n", g->views[i].zoom); */
    }
}
static void Game_updateCountdown(Game *g) {

    size_t i;
    for(i=0 ; i<g->view_count ; ++i) {
        if(g->input.players[i].zooming_in && g->views[i].zoom < 6.f)
            g->views[i].zoom *= 1.1f;
        if(g->input.players[i].zooming_out && g->views[i].zoom > 0.02f)
            g->views[i].zoom *= .9f;
    }

    uint32_t new_ms = SDL_GetTicks();
    g->race_time_ms += new_ms - g->race_step_ms;
    g->race_step_ms = new_ms;
    if(1-g->race_time_ms > 0) {
        printf("%d...\n", 1-g->race_time_ms/1000);
        return;
    }
    puts("Go!!!");
    g->update = Game_updateRace;
    g->update(g);
}
static void Game_updatePreCountdown(Game *g) {
    size_t i;
    for(i=0 ; i<g->view_count ; ++i) {
        g->views[i].center.x = g->ships[i].pos.x;
        g->views[i].center.y = g->ships[i].pos.y;
        g->views[i].tilt = g->ships[i].tilt;
        g->views[i].ortho_right = 8.f;
    }
    for(i=0 ; i<g->ship_count ; ++i) {
        g->ships[i].guides[0].pos.x = g->map.checkpoints[0].pos.x;
        g->ships[i].guides[0].pos.y = g->map.checkpoints[0].pos.y;
        g->ships[i].guides[0].scale.x = .6f;
        g->ships[i].guides[0].scale.y = .6f;
        g->ships[i].guides[0].r = 0;
        g->ships[i].guides[0].g = 0;
        g->ships[i].guides[0].b = 0;
        const size_t j = (i+1)%(g->ship_count);
        g->ships[i].guides[1].pos.x = g->ships[j].pos.x;
        g->ships[i].guides[1].pos.y = g->ships[j].pos.y;
        g->ships[i].guides[1].scale.x = .4f;
        g->ships[i].guides[1].scale.y = .4f;
        g->ships[i].guides[1].r = g->ships[j].r;
        g->ships[i].guides[1].g = g->ships[j].g;
        g->ships[i].guides[1].b = g->ships[j].b;
        g->ships[i].guide_count = 2;
        Ship_refreshGuides(g->ships+i);
    }
    g->race_time_ms = -3000;
    g->race_step_ms = SDL_GetTicks();

    g->update = Game_updateCountdown;
    g->update(g);
}
static void Game_updateMapSelection(Game *g) {
    memset(&g->map, 0, sizeof(Map));
    g->map.size.x = 2000.f;
    g->map.size.y = 1000.f;
    g->map.friction = 0.99999f;
    g->map.checkpoint_count = 3;
    g->map.checkpoints[0].pos.x = 250.f;
    g->map.checkpoints[0].pos.y = 50.f;
    g->map.checkpoints[0].radius = 20.f;
    g->map.checkpoints[1].pos.x = -20.f;
    g->map.checkpoints[1].pos.y = 200.f;
    g->map.checkpoints[1].radius = 30.f;
    g->map.checkpoints[2].pos.x = 0.f;
    g->map.checkpoints[2].pos.y = -50.f;
    g->map.checkpoints[2].radius = 40.f;
    g->map.checkpoint_count = 3;
    g->ships[0].pos.x = -1.f;
    g->ships[1].pos.x =  1.f;

    g->update = Game_updatePreCountdown;
    g->update(g);
}
static void Game_updateShipSelection(Game *g) {
    g->update = Game_updateMapSelection;
    memset(g->ships, 0, g->ship_count*sizeof(Ship));
    g->ships[0].accel_multiplier = 0.01f;
    g->ships[0].tilt_step = M_PI/45.f;
    g->ships[0].friction = 0.997f;
    g->ships[0].max_speed = 200.f;
    memcpy(g->ships+1, g->ships, sizeof(Ship));
    g->ships[0].r = 1.f;
    g->ships[1].b = 1.f;
    g->update(g);
}
static void Game_updateMainMenu(Game *g) { 
    g->update = Game_updateShipSelection;
    g->ship_count = 2;
    g->update(g);
}
static void Game_updateStartScreen(Game *g) {
    g->update = Game_updateMainMenu;
    g->update(g);
}
void Game_update(Game *g) {
    g->update(g);
    /* L'ordre compte. S'assurer que g->update(g) soit d'abord. */
    switch(g->view_count) {
    case 1:
        View_mapPixelToCoords(g->views, &g->world_mouse_cursor, 
                             &g->input.old_mouse_pos);
        break;
    case 2:
        if(g->input.old_mouse_pos.x < g->window_size.x/2) {
            View_mapPixelToCoords(g->views, &g->world_mouse_cursor, 
                                 &g->input.old_mouse_pos);
            break;
        }
        View_mapPixelToCoords(g->views+1, &g->world_mouse_cursor, 
                             &g->input.old_mouse_pos);
        break;
    }
}


void Game_init(Game *g) {
    memset(g, 0, sizeof(*g));
    g->window_size.x = 800;
    g->window_size.y = 600;
    g->bits_per_pixel = 32;
    g->update = Game_updateStartScreen;
    memset(g->views, 0, 2*sizeof(View));
    g->views[0].zoom = 1.f;
    g->views[1].zoom = 1.f;
    g->view_count = 2;
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


