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
#include <Monospace.h>
#include <Game.h>

void Game_resizeViewports(Game *g) {
    g->menu_view.viewport_pos.x = 0;
    g->menu_view.viewport_pos.y = 0;
    g->menu_view.viewport_size.x = g->window_size.x;
    g->menu_view.viewport_size.y = g->window_size.y;
    switch(g->race.view_count) {
    case 1: 
        g->race.views[0].viewport_pos.x  = 0;
        g->race.views[0].viewport_pos.y  = 0;
        g->race.views[0].viewport_size.x = g->window_size.x;
        g->race.views[0].viewport_size.y = g->window_size.y;
        break;
    case 2: 
        g->race.views[0].viewport_pos.x  = 0;
        g->race.views[0].viewport_pos.y  = 0;
        {
            const uint32_t hw = g->window_size.x/2;
            g->race.views[0].viewport_size.x = hw;
            g->race.views[0].viewport_size.y = g->window_size.y;
            g->race.views[1].viewport_pos.x  = hw;
            g->race.views[1].viewport_pos.y  = 0;
            g->race.views[1].viewport_size.x = hw;
            g->race.views[1].viewport_size.y = g->window_size.y;
        }
        break;
    }
}

static void Game_resizeSprites(Game *g) {
    /* resizeViewports() a été appelée avant. */
    const float top = View_getOrthoTop(&g->menu_view), height = 2.f*top;
    Sprite_resizeToWidth(&g->main_menu.sky, 2.f);
    size_t i;
    for(i=0 ; i<4 ; ++i)
        Sprite_resizeToWidth(&g->main_menu.bg[i], 2.f);
    Sprite_resizeToHeight(&g->main_menu.title, height*120.f/1200.f);
    Sprite_resizeToHeight(&g->main_menu.oneplayer, height*45.f/1200.f);
    Sprite_resizeToHeight(&g->main_menu.twoplayers, height*45.f/1200.f);
    for(i=0 ; i<MAX_PLAYERS ; ++i)
        Sprite_resizeToHeight(&g->ship_menu.titles[i], height*50.f/1200.f);
    Sprite_resizeToHeight(&g->ship_menu.ship_cell, height*200.f/1200.f);
    Sprite_resizeToHeight(&g->map_menu.left_arrow, height*140.f/1200.f);
    Sprite_resizeToWidth(&g->clap_transition.top, 2.f*g->menu_view.ortho_right);
    Sprite_resizeToWidth(&g->clap_transition.bottom, 2.f*g->menu_view.ortho_right);
    size_t p;
    for(i=0 ; i<MAX_SHIPS ; ++i) {
        for(p=0 ; p<MAX_PALETTES ; ++p) {
            Sprite_resizeToHeight(&g->ship_data[i].banner[p], height*140.f/1200.f);
            Sprite_resizeToHeight(&g->ship_data[i].icon[p], height*200.f/1200.f);
            Sprite_resizeToHeight(&g->ship_data[i].side[p], height*300.f/1200.f);
            Sprite_resizeToHeight(&g->ship_data[i].artwork[p], height*840.f/1200.f);
        }
    }
    for(i=0 ; i<g->map_data_count ; ++i) {
        Sprite_resizeToHeight(&g->map_data[i].banner, height*140.f/1200.f);
        Sprite_resizeToHeight(&g->map_data[i].artwork, height*840.f/1200.f);
    }
}

void Game_reshape(Game *g) {

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
    glBlendColor(1.f, 1.f, 1.f, 1.f);

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
    Game_resizeSprites(g);
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

static void Game_renderRaceView(const Game *g, size_t view_index) {
    const View *v = g->race.views + view_index;

    View_apply(v);

    Map_render(&g->race.map);
    size_t i;
    for(i=0 ; i<g->race.ship_count ; ++i) {
        Ship_render(g->race.ships+i);
        if(i==view_index) {
            Map_renderCheckpoints(&g->race.map, g->race.ships[i].next_checkpoint_index);
            Ship_renderGuides(g->race.ships+i);
        }
    }
    Game_renderWorldMouseCursor(g);

    View gui_view;
    gui_view.zoom = 1.f;
    gui_view.ortho_right = 1.f;
    gui_view.center.x = 0.f;
    gui_view.center.y = 0.f;
    gui_view.tilt = -M_PI/2.f;
    gui_view.viewport_pos = v->viewport_pos;
    gui_view.viewport_size = v->viewport_size;
    gui_view.sym.x = gui_view.sym.y = -1.f;
    char str[16];
    snprintf(str, sizeof(str), "speed: %.6f", 
            sqrtf(
                g->race.ships[view_index].physic_solid.speed.x*g->race.ships[view_index].physic_solid.speed.x
              + g->race.ships[view_index].physic_solid.speed.y*g->race.ships[view_index].physic_solid.speed.y
            ));
    View_apply(&gui_view);
    glPushMatrix();
    glTranslatef(.8f-Monospace_getStringWidth(str, .1f)*.5f, -View_getOrthoTop(&gui_view)+.1f, 0.f);
    Monospace_renderCenteredString(str, .1f);
    glPopMatrix();
}

void Game_renderRace(const Game *g) {
    size_t i;
    for(i=0 ; i<g->race.view_count ; ++i)
        Game_renderRaceView(g, i);

    View_apply(&g->menu_view);
    char str[64];
    if(g->race.time_of_completion) {
        snprintf(str, sizeof(str), "Player %u won!", g->race.rankings[0]+1);
        Monospace_renderCenteredString(str, .1f);
        return;
    }
    if(g->race.time_ms >= 0) {
        snprintf(str, sizeof(str), "%02u:%02u:%03u", 
                (g->race.time_ms/1000)/60, 
                (g->race.time_ms/1000)%60, 
                (g->race.time_ms%1000));
        glPushMatrix();
        glTranslatef(0.f, View_getOrthoTop(&g->menu_view)-.1f, 0.f);
        Monospace_renderCenteredString(str, .1f);
        glPopMatrix();
    } else if(g->race.time_ms >= -3000) {
        snprintf(str, sizeof(str), "%d", 1+(-g->race.time_ms)/1000);
        Monospace_renderCenteredString(str, .3f);
    }
}

static void ClapTransition_render(const ClapTransition *ct) {
    glPushMatrix();
    glTranslatef(ct->top_pos.x, ct->top_pos.y, 0.f);
    Sprite_render(&ct->top);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(ct->bottom_pos.x, ct->bottom_pos.y, 0.f);
    Sprite_render(&ct->bottom);
    glPopMatrix();
}
void Game_renderRaceWithClap(const Game *g) {
    Game_renderRace(g);
    View_apply(&g->menu_view);
    ClapTransition_render(&g->clap_transition);
}

void Game_renderMapMenuWithClap(const Game *g) {
    Game_renderMapMenu(g);
    View_apply(&g->menu_view); /* C'est pas grave si renderMapMenu() la réapplique. */
    ClapTransition_render(&g->clap_transition);
}
static void Game_renderBg(const Game *g, float top) {
    Sprite_render(&g->main_menu.sky);
    glPushMatrix(); {
        glTranslatef(0, -top+g->main_menu.bg[0].half_size.y, 0.f);
        Sprite_render(&g->main_menu.bg[0]);
    } glPopMatrix();
}

void Game_renderMapMenu(const Game *g) {
    View_apply(&g->menu_view);
    const float top = View_getOrthoTop(&g->menu_view), height = 2.f*top;
    Game_renderBg(g, top);
    Sprite_bindAlpha(g->fade_transition.alpha);
    glPushMatrix(); {
        glTranslatef(-g->map_data[g->map_menu.selected_map_index].banner.half_size.x - g->map_menu.left_arrow.half_size.x, 
                top-height*160.f/1200.f, 0.f);
        Sprite_render(&g->map_menu.left_arrow);
    } glPopMatrix();
    glPushMatrix(); {
        glTranslatef(g->map_data[g->map_menu.selected_map_index].banner.half_size.x + g->map_menu.left_arrow.half_size.x, 
                top-height*160.f/1200.f, 0.f);
        glScalef(-1.f, 1.f, 1.f);
        Sprite_render(&g->map_menu.left_arrow);
    } glPopMatrix();
    glPushMatrix(); {
        glTranslatef(0.f, top-height*160.f/1200.f, 0.f);
        Sprite_render(&g->map_data[g->map_menu.selected_map_index].banner);
    } glPopMatrix();
    glPushMatrix(); {
        glTranslatef(0.f, top-height*720.f/1200.f, 0.f);
        Sprite_render(&g->map_data[g->map_menu.selected_map_index].artwork);
    } glPopMatrix();
    Sprite_bindAlpha(1.f);
}

void Game_renderShipMenu(const Game *g) {
    View_apply(&g->menu_view);
    const float top = View_getOrthoTop(&g->menu_view), height = 2.f*top;
    const float right = g->menu_view.ortho_right, width = 2.f*right;
    Game_renderBg(g, top);
    Sprite_bindAlpha(g->fade_transition.alpha);
    glPushMatrix(); {
        glTranslatef(.0f, top-height*70.f/1200.f, .0f);
        Sprite_render(&g->ship_menu.titles[g->ship_menu.player_index]);
    } glPopMatrix();

    const Vec2 ship_cell_pos[MAX_SHIPS] = {
        { -right+width*210.f/1600.f, top-height*340.f/1200.f },
        { -right+width*466.f/1600.f, top-height*340.f/1200.f },
        { -right+width*210.f/1600.f, top-height*570.f/1200.f },
        { -right+width*466.f/1600.f, top-height*570.f/1200.f }
    };
    const Vec2 pal_cell_pos[MAX_PALETTES] = {
        { width*-530.f/1600.f, top-height*1080.f/1200.f },
        { width*-320.f/1600.f, top-height*1080.f/1200.f },
        { width*-110.f/1600.f, top-height*1080.f/1200.f },
        { width* 110.f/1600.f, top-height*1080.f/1200.f },
        { width* 320.f/1600.f, top-height*1080.f/1200.f },
        { width* 530.f/1600.f, top-height*1080.f/1200.f }
    };
    Vec2 cursor_pos;
    if(g->update == Game_updatePaletteSelection)
        cursor_pos = pal_cell_pos[g->ship_menu.selected_pal_index[g->ship_menu.player_index]];
    else
        cursor_pos = ship_cell_pos[g->ship_menu.selected_ship_index[g->ship_menu.player_index]];

    if(g->ship_menu.player_index)
        glColor3f(0.f, 0.f, 1.f);
    else
        glColor3f(1.f, 0.f, 0.f);

    glPushMatrix();
    {
        glTranslatef(cursor_pos.x, cursor_pos.y, 0.f);
        glScalef(top*198.f/1200.f, top*198.f/1200.f, 1.f);
        renderCircle(32, false);
        size_t thickness;
        for(thickness=0 ; thickness<10 ; ++thickness) {
            glScalef(1.01f, 1.01f, 1.f);
            renderCircle(32, false);
        }
    }
    glPopMatrix();

    size_t i;
    for(i=0 ; i<MAX_SHIPS ; ++i) {
        glPushMatrix();
        glTranslatef(ship_cell_pos[i].x, ship_cell_pos[i].y, 0.f);
        Sprite_render(&g->ship_menu.ship_cell);
        Sprite_render(&g->ship_data[g->ship_menu.selected_ship_index[g->ship_menu.player_index]].icon[0]);
        glPopMatrix();
    }
    for(i=0 ; i<MAX_PALETTES ; ++i) {
        glPushMatrix();
        glTranslatef(pal_cell_pos[i].x, pal_cell_pos[i].y, 0.f);
        Sprite_render(&g->ship_menu.ship_cell);
        Sprite_render(&g->ship_data[g->ship_menu.selected_ship_index[g->ship_menu.player_index]].icon[i]);
        glPopMatrix();
    }

#define RENDER_SHIP_ELEMENT(elt, px, py) \
    glPushMatrix(); \
    { \
        glTranslatef(px, py, 0.f); \
        Sprite_render(&g->ship_data[g->ship_menu.selected_ship_index[g->ship_menu.player_index]].elt[g->ship_menu.selected_pal_index[g->ship_menu.player_index]]); \
    } \
    glPopMatrix();
    RENDER_SHIP_ELEMENT(artwork, -right+width*1155/1600.f, top-height*550.f/1200.f)
    RENDER_SHIP_ELEMENT(banner, -right+width*355/1600.f, top-height*840.f/1200.f)
#undef RENDER_SHIP_ELEMENT

    Sprite_bindAlpha(1.f);
}

void Game_renderMainMenu(const Game *g) {
    View_apply(&g->menu_view);
    const float top = View_getOrthoTop(&g->menu_view), height = 2.f*top;
    Game_renderBg(g, top);
    Sprite_bindAlpha(g->fade_transition.alpha);
    glPushMatrix(); {
        glTranslatef(0, top-height*320.f/1200.f, 0.f);
        Sprite_render(&g->main_menu.title);
    } glPopMatrix();

    glPushMatrix(); {
        glTranslatef(0, top-height*600.f/1200.f, 0.f);
        Sprite_render(&g->main_menu.oneplayer);
        if(g->player_count==1) {
            glTranslatef(-1.2f*g->main_menu.oneplayer.half_size.x, 0.f, 0.f);
            glRotatef(270,0,0,1);
            glScalef(g->main_menu.twoplayers.half_size.y, g->main_menu.twoplayers.half_size.y, 1.f);
            glColor4f(1.f, 0.f, 0.f, 1.f);
            renderIsocelesTriangle(true);
        }
    } glPopMatrix();
    glPushMatrix(); {
        glTranslatef(0, top-height*700.f/1200.f, 0.f);
        Sprite_render(&g->main_menu.twoplayers);
        if(g->player_count==2) {
            glTranslatef(-1.2f*g->main_menu.twoplayers.half_size.x, 0.f, 0.f);
            glRotatef(270,0,0,1);
            glScalef(g->main_menu.twoplayers.half_size.y, g->main_menu.twoplayers.half_size.y, 1.f);
            glColor4f(1.f, 0.f, 0.f, 1.f);
            renderIsocelesTriangle(true);
        }
    } glPopMatrix();
    Sprite_bindAlpha(1.f);
}

void Game_renderStartScreen(const Game *g) { /* Rien pour l'instant. */ }
void Game_render(const Game *g) {
    g->render(g);
}
