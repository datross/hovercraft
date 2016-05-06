#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <SDL/SDL.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <Utils.h>
#include <Monospace.h>
#include <Sprite.h>
#include <Game.h>
#include <unistd.h> /* chdir() */

void Game_init(Game *g) {
    memset(g, 0, sizeof(*g));
    g->window_size.x = 800;
    g->window_size.y = 600;
    g->bits_per_pixel = 32;
    g->update = Game_updateStartScreen;
    g->render = Game_renderStartScreen;
    g->player_count = 1;
    g->menu_view.tilt = 0;
    g->menu_view.zoom = 1.f;
    g->menu_view.ortho_right = 1.f;
    g->clap_transition.update = ClapTransition_updateDummy;
    g->fade_transition.update = FadeTransition_updateDummy;
    g->fade_transition.alpha = 1.f;
    char *expath = Util_getExecutablePath();
    *strrchr(expath, '/') = '\0';
    if(chdir(expath)==-1)
        perror("chdir()");
    free(expath);
    if(chdir("../")==-1)
        perror("chdir()");
    /* Il faut initialiser opengl avant de charger les textures. */
    Game_reshape(g);
    Monospace_setup();
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    Monospace_renderCenteredString("Loading...", .1f);
    Sprite_bindAlpha(1.f);
    SDL_GL_SwapBuffers();
    puts("Chargement des menus...");
    Game_loadMenus(g, "res/menus");
    puts("Chargement des sons et musiques...");
    Game_loadSounds(g, "res/menus", "res/snd");
    puts("Chargement des bateaux...");
    Game_loadShips(g, "res/ships");
    puts("Chargement des maps...");
    Game_loadMaps(g, "res/maps");
    Game_reshape(g);
}
void Game_deinit(Game *g) {    
    glDeleteTextures(1, &g->main_menu.sky.tex_id);
    size_t i, p;
    for(i=0 ; i<MAX_SHIPS ; ++i)
        for(p=0 ; p<MAX_PALETTES ; ++p) {
            glDeleteTextures(1, &g->ship_data[i].icon[p].tex_id);
            glDeleteTextures(1, &g->ship_data[i].artwork[p].tex_id);
        }
    for(i=0 ; i<g->map_data_count ; ++i)
        MapData_free(&(g->map_data[i]));
    Monospace_cleanup();
    Mix_FreeMusic(g->main_music);
    Mix_FreeChunk(g->snd_menu_transition);
    Mix_FreeChunk(g->clap_transition.snd_clap_close);
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
        case SDLK_ESCAPE: g->input.players[0].now.escaping      = true;
                          g->input.players[1].now.escaping      = true; break;
        case SDLK_SPACE:  g->input.players[0].now.accelerating  = true; break;
        case SDLK_UP:     g->input.players[0].now.zooming_in    = true; break;
        case SDLK_DOWN:   g->input.players[0].now.zooming_out   = true; break;
        case SDLK_LEFT:   g->input.players[0].now.left_tilting  = true; break;
        case SDLK_RIGHT:  g->input.players[0].now.right_tilting = true; break;
        case SDLK_LSHIFT: g->input.players[1].now.accelerating  = true; break;
        case SDLK_z:      g->input.players[1].now.zooming_in    = true; break;
        case SDLK_s:      g->input.players[1].now.zooming_out   = true; break;
        case SDLK_q:      g->input.players[1].now.left_tilting  = true; break;
        case SDLK_d:      g->input.players[1].now.right_tilting = true; break;
        }
        break;
    case SDL_KEYUP:
        switch(e->key.keysym.sym) {
        case SDLK_ESCAPE: g->input.players[0].now.escaping      = false;
                          g->input.players[1].now.escaping      = false; break;
        case SDLK_SPACE:  g->input.players[0].now.accelerating  = false; break;
        case SDLK_UP:     g->input.players[0].now.zooming_in    = false; break;
        case SDLK_DOWN:   g->input.players[0].now.zooming_out   = false; break;
        case SDLK_LEFT:   g->input.players[0].now.left_tilting  = false; break;
        case SDLK_RIGHT:  g->input.players[0].now.right_tilting = false; break;
        case SDLK_LSHIFT: g->input.players[1].now.accelerating  = false; break;
        case SDLK_z:      g->input.players[1].now.zooming_in    = false; break;
        case SDLK_s:      g->input.players[1].now.zooming_out   = false; break;
        case SDLK_q:      g->input.players[1].now.left_tilting  = false; break;
        case SDLK_d:      g->input.players[1].now.right_tilting = false; break;

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

