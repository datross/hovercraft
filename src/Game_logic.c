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
#include <Ship.h>

void FadeTransition_updateDummy(Game *g) { /* Rien, et c'est le but. */ }

static void FadeTransition_update(Game *g) {
    g->fade_transition.alpha += g->fade_transition.alpha_vel;
    if(g->fade_transition.alpha < 0.f) {
        g->fade_transition.alpha = 0.f;
        g->fade_transition.alpha_vel = -g->fade_transition.alpha_vel;
        g->render = g->fade_transition.next_render_func;
    } else if(g->fade_transition.alpha > 1.f) {
        g->fade_transition.alpha = 1.f;
        g->fade_transition.update = FadeTransition_updateDummy;
    }
}

static void FadeTransition_fadeTo(FadeTransition *ft, void (*render)(const Game *)) {
    ft->next_render_func = render;
    ft->alpha_vel = -FADE_TRANSITION_INC;
    ft->update = FadeTransition_update;
}

void ClapTransition_updateDummy(ClapTransition *ct, float top) { 
    /* Rien, et c'est le but. */ 
}
static void ClapTransition_updateOpen(ClapTransition *ct, float top) {
    const float inc = top*.06f;
    ct->top_pos.y += inc;
    ct->bottom_pos.y -= inc;
    if(ct->top_pos.y - ct->top.half_size.y > top)
        ct->update = ClapTransition_updateDummy;
}
static void ClapTransition_updateStayClosed(ClapTransition *ct, float top) {
    if(SDL_GetTicks() - ct->time_of_closing >= 1400)
        ct->update = ClapTransition_updateOpen;
    /* On pourrait meme faire un effet de shaking. */
}

static void ClapTransition_updateClose(ClapTransition *ct, float top) {
    const float inc = top*.06f;
    ct->top_pos.y -= inc;
    ct->bottom_pos.y += inc;
    if(ct->top_pos.y - ct->top.half_size.y < 0.f) {
        ct->top_pos.y = ct->top.half_size.y;
        ct->bottom_pos.y = -ct->bottom.half_size.y;
        ct->update = ClapTransition_updateStayClosed;
        ct->time_of_closing = SDL_GetTicks();
        Mix_PlayChannel(-1, ct->snd_clap_close, 0);
    }
}

static void ClapTransition_start(ClapTransition *ct, float top) {
    ct->top_pos.x = ct->bottom_pos.x = 0.f;
    ct->top_pos.y = top + ct->top.half_size.y;
    ct->bottom_pos.y = -top - ct->bottom.half_size.y;
    ct->update = ClapTransition_updateClose;
}

static void Game_updateMainMenu(Game *g);
static void Game_updateMapMenu(Game *g);

static void Game_updateRaceToMapMenu(Game *g) {
    if(g->clap_transition.update == ClapTransition_updateDummy) {
        g->update = Game_updateMapMenu;
    }
}

static void Game_updatePostRace(Game *g) { 
    size_t i;
    World_clean(&(g->race.world));
    for(i=0 ; i<g->race.ship_count ; ++i) {
        Ship *s = g->race.ships+i;
        Ship_deinit(s);
    }
    //Mix_HaltMusic();
    Mix_VolumeMusic(MIX_MAX_VOLUME);
    Mix_PlayMusic(g->main_music, -1);
    g->render = Game_renderMapMenuWithClap;
    g->update = Game_updateRaceToMapMenu;
}

static inline void reactToZoom(Game *g, size_t i) {
    if(g->input.players[i].now.zooming_in && g->race.views[i].zoom < 6.f)
        g->race.views[i].zoom *= 1.1f;
    if(g->input.players[i].now.zooming_out && g->race.views[i].zoom > 0.02f)
        g->race.views[i].zoom *= .9f;
}

static void Game_updateRace(Game *g) { 

    uint32_t new_ms = SDL_GetTicks();
    g->race.time_ms += new_ms - g->race.step_ms;
    g->race.step_ms = new_ms;

    if(g->race.time_of_completion) {
        Mix_VolumeMusic(Mix_VolumeMusic(-1)-1);
        if(g->clap_transition.update == ClapTransition_updateStayClosed) {
            g->update = Game_updatePostRace;
        } else if(new_ms-g->race.time_of_completion > 2400 
               && g->clap_transition.update != ClapTransition_updateClose) {
            ClapTransition_start(&g->clap_transition, View_getOrthoTop(&g->menu_view));
            g->render = Game_renderRaceWithClap;
        }
    }
    Process_physics(&(g->race.world), g->tickrate);
    size_t i;
    for(i=0 ; i<g->race.ship_count ; ++i) {
        Ship *s = g->race.ships+i;
        
        Vec2 ship_direction = MakeVec2(cos(s->physic_solid.rotation),
                                       sin(s->physic_solid.rotation));
        s->particle_system_reactor.source_position = LocalToGlobal2(s->data->reactor_position,
                                            s->physic_solid.position, s->physic_solid.rotation);
        s->particle_system_reactor.source_speed = AddVec2(s->physic_solid.speed,
                        MulVec2(OrthogonalVec2(ship_direction), - s->physic_solid.rotation_speed * s->data->reactor_distance_to_center));
        s->particle_system_reactor.emit_direction =  MulVec2(ship_direction, -0.2 * REACTOR_PARTICLES_SPEED);
        s->particle_system_reactor.particle_size = REACTOR_PARTICLES_SIZE * g->race.views[i].zoom;
        if(g->input.players[i].now.accelerating) {
            s->main_translate_force.force.x = s->data->accel_multiplier*cosf(s->physic_solid.rotation);
            s->main_translate_force.force.y = s->data->accel_multiplier*sinf(s->physic_solid.rotation);
            World_addForce(&(g->race.world), &(s->main_translate_force));
            
            s->particle_system_reactor.emit_direction.x *= 5.;
            s->particle_system_reactor.emit_direction.y *= 5.;
        } 

        if(g->input.players[i].now.left_tilting) {
			s->main_rotate_force.force.x = s->data->tilt_step;
			World_addForce(&(g->race.world), &(s->main_rotate_force));
        }
        if(g->input.players[i].now.right_tilting) {
			s->main_rotate_force.force.x = - s->data->tilt_step;
			World_addForce(&(g->race.world), &(s->main_rotate_force));
        }

        /* La friction joue beaucoup sur la vitesse maximale
         * factuellement atteignable des véhicules. */
        
        s->main_translation_friction.force = MulVec2(s->physic_solid.speed, -s->data->friction);
        s->main_rotation_friction.force.x = -s->data->friction * s->physic_solid.rotation_speed;
 
        World_addForce(&(g->race.world), &(s->main_translation_friction));
        World_addForce(&(g->race.world), &(s->main_rotation_friction));

        const float dist = sqrtf(
              s->physic_solid.speed.x*s->physic_solid.speed.x 
            + s->physic_solid.speed.y*s->physic_solid.speed.y
        );
        const float speed = 1000.f*dist/g->tickrate;
        if(speed > s->data->max_speed) {
            printf("Reaching max speed.\n");
            /* FIXME C'est une approximation qui est précise
             * avec 'max_speed' élevée, mais pas tant que ça
             * quand elle est faible. */
            /*s->vel.x *= s->max_speed/speed;
            s->vel.y *= s->max_speed/speed;*/
        }
        
        ParticleSystem_compute_step(&(s->particle_system_reactor), g->tickrate);
    }

    for(i=0 ; i<g->race.ship_count ; ++i) {
        Ship *s = g->race.ships+i;
        const Checkpoint *c = g->race.map.data->checkpoints + s->next_checkpoint_index;
        Vec2 diff = { 
            c->pos.x - s->physic_solid.position.x, 
            c->pos.y - s->physic_solid.position.y 
        };
        const float c_dist = sqrtf(diff.x*diff.x + diff.y*diff.y);
        if(c_dist <= c->radius)
            ++(s->next_checkpoint_index);

        const size_t j = (i+1)%(g->race.ship_count);
        s->guides[1].pos.x = g->race.ships[j].physic_solid.position.x;
        s->guides[1].pos.y = g->race.ships[j].physic_solid.position.y;
        Ship_refreshGuides(s);

        if(s->next_checkpoint_index >= g->race.map.data->checkpoint_count) {
            if(!g->race.time_of_completion) {
                g->race.time_of_completion = SDL_GetTicks();
                g->race.rankings[0] = i;
                g->race.rankings[1] = j;
                g->race.completion_times[i] = g->race.time_ms;
            }
        } else s->guides[0].pos = c->pos;

        /*
        printf("pos:(%f, %f); tilt: %f deg; speed: %u units/s\n", 
               s->pos.x, s->pos.y, degf(s->tilt), (uint32_t)speed);
        */

        if(i>=g->race.view_count)
            break;
        g->race.views[i].center = s->physic_solid.position;
        //g->views[i].tilt = atan(s->physic_solid.speed.y / s->physic_solid.speed.x);
        g->race.views[i].tilt = s->physic_solid.rotation-M_PI/2.f;
        reactToZoom(g, i);
        /* if(i==0)printf("zoom : %f\n", g->views[i].zoom); */
    }
}

static void Game_updateCountdown(Game *g) {
    if(g->clap_transition.update == ClapTransition_updateDummy)
        g->render = Game_renderRace; /* Sans clap. */
    size_t i;
    for(i=0 ; i<g->race.view_count ; ++i)
        reactToZoom(g, i);
    uint32_t new_ms = SDL_GetTicks();
    g->race.time_ms += new_ms - g->race.step_ms;
    g->race.step_ms = new_ms;
    int32_t ms_left = 1-g->race.time_ms;
    for(i=0 ; i<g->race.ship_count ; ++i) {
        Ship *s = g->race.ships + i;
        /* J'ai mouru un petit peu en écrivant ça. */
        if(ms_left <= 3000)
            s->above_index = 2;
        if(ms_left <= 2900)
            s->above_index = 3;
        if(ms_left <= 2800)
            s->above_index = 4;
        if(ms_left <= 2000)
            s->above_index = 5;
        if(ms_left <= 1900)
            s->above_index = 6;
        if(ms_left <= 1800)
            s->above_index = 7;
        if(ms_left <= 1000)
            s->above_index = 8;
        if(ms_left <=  900)
            s->above_index = 9;
        if(ms_left <=  800)
            s->above_index = 0;
    }
    if(ms_left <= 0) {
        puts("Go!!!");
        Mix_VolumeMusic(MIX_MAX_VOLUME);
        Mix_PlayMusic(g->race.map.data->music, -1);
        g->update = Game_updateRace;
        g->update(g);
        return;
    }
    if(ms_left/1000 == g->race.countdown_prev_seconds_left)
        return;
    g->race.countdown_prev_seconds_left = ms_left/1000;
    printf("%d...\n", 1+ms_left/1000);
}

static void Game_updatePreCountdown(Game *g) {
    g->race.view_count = g->player_count;
    g->race.ship_count = g->player_count;
    Game_resizeViewports(g);
    g->race.world.solids       = NULL;
    g->race.world.forces_head  = NULL;
    g->race.world.forces_tail  = NULL;
    g->race.map.data = g->map_data + g->map_menu.selected_map_index;
    for(unsigned i = 0; i < g->race.map.data->wall_count; ++i) {
        World_addObstacle(&(g->race.world), &(g->race.map.data->walls[i].physic_obstacle));
    }
    g->race.time_of_completion = 0;
    size_t i;
    for(i=0 ; i<g->race.ship_count ; ++i) {
        g->race.rankings[i] = 0;
        g->race.completion_times[i] = 0;
        Ship_init(g->race.ships + i);
        World_addSolid(&(g->race.world), &(g->race.ships[i].physic_solid)); 
        g->race.ships[i].physic_solid.position.x = g->race.map.data->start[i].pos.x;
        g->race.ships[i].physic_solid.position.y = g->race.map.data->start[i].pos.y;
        g->race.ships[i].physic_solid.rotation = radf(g->race.map.data->start[i].tilt);
        g->race.ships[i].data = g->ship_data + g->ship_menu.selected_ship_index[i];
        g->race.ships[i].palette_index = g->ship_menu.selected_pal_index[i];
        g->race.ships[i].above_index = 1;
        g->race.ships[i].next_checkpoint_index = 0;
        g->race.ships[i].guides[0].pos.x = g->race.map.data->checkpoints[0].pos.x;
        g->race.ships[i].guides[0].pos.y = g->race.map.data->checkpoints[0].pos.y;
        g->race.ships[i].guides[0].scale.x = .6f;
        g->race.ships[i].guides[0].scale.y = .6f;
        g->race.ships[i].guides[0].r = 0;
        g->race.ships[i].guides[0].g = 0;
        g->race.ships[i].guides[0].b = 0;
        g->race.ships[i].particle_system_reactor.particle_color.r = 1.0;
        g->race.ships[i].particle_system_reactor.particle_color.g = 1.0;
        g->race.ships[i].particle_system_reactor.particle_color.b = 0.8;
        g->race.ships[i].particle_system_reactor.source_position = LocalToGlobal2(g->race.ships[i].data->reactor_position,
                                            g->race.ships[i].physic_solid.position, g->race.ships[i].physic_solid.rotation);
        g->race.ships[i].particle_system_reactor.emit_direction = MakeVec2(cos(g->race.ships[i].physic_solid.rotation),
                                                                            cos(g->race.ships[i].physic_solid.rotation));
        g->race.ships[i].particle_system_reactor.source_speed = MakeVec2(0,0);
        g->race.ships[i].particle_system_reactor.source_radius_max = 0.02;
        g->race.ships[i].particle_system_reactor.particle_size = REACTOR_PARTICLES_SIZE;
        g->race.ships[i].particle_system_reactor.particle_size_dispersion = 2;
        g->race.ships[i].particle_system_reactor.emit_direction_dispersion = 0.7;
        g->race.ships[i].particle_system_reactor.emit_speed_dispersion = 1.;
        g->race.ships[i].particle_system_reactor.life_time_max = 210;
        g->race.ships[i].particle_system_reactor.life_time_dispersion = 1.;
        g->race.ships[i].particle_system_reactor.color_dispersion = 0.2;
        g->race.ships[i].particle_system_reactor.dampness = 1;
        ParticleSystem_init_particles(&(g->race.ships[i].particle_system_reactor));
        if(g->player_count == 2) {
            const size_t j = (i+1)%(g->race.ship_count);
            g->race.ships[i].guides[1].pos.x = g->race.ships[j].physic_solid.position.x;
            g->race.ships[i].guides[1].pos.y = g->race.ships[j].physic_solid.position.y;
            g->race.ships[i].guides[1].scale.x = .4f;
            g->race.ships[i].guides[1].scale.y = .4f;
            g->race.ships[i].guides[1].r = j ? 0.f : 1.f;
            g->race.ships[i].guides[1].g = 0.f;
            g->race.ships[i].guides[1].b = j ? 1.f : 0.f;
        }
        g->race.ships[i].guide_count = g->player_count;
        Ship_refreshGuides(g->race.ships + i);
    }
    for(i=0 ; i<g->race.view_count ; ++i) {
        g->race.views[i].center.x = g->race.ships[i].physic_solid.position.x;
        g->race.views[i].center.y = g->race.ships[i].physic_solid.position.y;
        g->race.views[i].tilt = g->race.ships[i].physic_solid.rotation-M_PI/2.f;
        g->race.views[i].ortho_right = 8.f;
        g->race.views[i].zoom = 2.f;
    }
    g->race.time_ms = -5000;
    g->race.step_ms = SDL_GetTicks();

    g->update = Game_updateCountdown;
    g->update(g);
}
static void Game_updatePostMapMenu(Game *g) {
    if(g->clap_transition.update != ClapTransition_updateClose)
        g->render = Game_renderRaceWithClap;
    if(g->clap_transition.update == ClapTransition_updateOpen
    || g->clap_transition.update == ClapTransition_updateDummy) {
        g->update = Game_updatePreCountdown;
        Mix_HaltMusic();
    }
    Mix_VolumeMusic(Mix_VolumeMusic(-1)-1);
}
static void Game_updateShipMenu(Game *g);
static void Game_updateMapMenu(Game *g) {
#define p_input (g->input.players[0])
#define p_map (g->map_menu.selected_map_index)
#define map_cnt (g->map_data_count)
    if(PLAYERINPUT_PRESSED(p_input, left_tilting)
    || PLAYERINPUT_PRESSED(p_input, zooming_out)) {
        if(p_map <= 0)
            p_map = map_cnt-1;
        else
            --p_map;
    }
    if(PLAYERINPUT_PRESSED(p_input, right_tilting)
    || PLAYERINPUT_PRESSED(p_input, zooming_in)) {
        ++p_map;
          p_map %= g->map_data_count;
    }
    if(PLAYERINPUT_PRESSED(p_input, escaping)) {
        Mix_PlayChannel(-1, g->snd_menu_transition, 0);
        g->update = Game_updateShipMenu;
        FadeTransition_fadeTo(&g->fade_transition, Game_renderShipMenu);
    }
    if(PLAYERINPUT_PRESSED(p_input, accelerating)) {
        g->update = Game_updatePostMapMenu;
        g->render = Game_renderMapMenuWithClap;
        ClapTransition_start(&g->clap_transition, View_getOrthoTop(&g->menu_view));
    }
#undef p_input
#undef p_map
#undef map_cnt
}
void Game_updatePaletteSelection(Game *g) {
#define p (g->ship_menu.player_index)
#define p_input (g->input.players[p])
#define p_pal  (g->ship_menu.selected_pal_index[p])
    if(PLAYERINPUT_PRESSED(p_input, left_tilting)
    || PLAYERINPUT_PRESSED(p_input, zooming_out)) {
        if(p_pal <= 0)
            p_pal = MAX_PALETTES-1;
        else
            --p_pal;
    }
    if(PLAYERINPUT_PRESSED(p_input, right_tilting)
    || PLAYERINPUT_PRESSED(p_input, zooming_in)) {
        ++p_pal;
          p_pal %= MAX_PALETTES;
    }
    if(PLAYERINPUT_PRESSED(p_input, accelerating)) {
        if(p+1 < g->player_count) {
            ++p;
            g->update = Game_updateShipMenu;
        } else {
            Mix_PlayChannel(-1, g->snd_menu_transition, 0);
            g->update = Game_updateMapMenu;
            FadeTransition_fadeTo(&g->fade_transition, Game_renderMapMenu);
        }
    }
    if(PLAYERINPUT_PRESSED(p_input, escaping))
        g->update = Game_updateShipMenu;
#undef p
#undef p_input
#undef p_pal
}
static void Game_updateShipMenu(Game *g) {
#define p (g->ship_menu.player_index)
#define p_input (g->input.players[p])
#define p_ship (g->ship_menu.selected_ship_index[p])
    if(PLAYERINPUT_PRESSED(p_input, left_tilting))
        if(p_ship==1 || p_ship==3)
            --p_ship;
    if(PLAYERINPUT_PRESSED(p_input, right_tilting))
        if(p_ship==0 || p_ship==2)
            ++p_ship;
    if(PLAYERINPUT_PRESSED(p_input, zooming_in))
        if(p_ship==2 || p_ship==3)
            p_ship -= 2;
    if(PLAYERINPUT_PRESSED(p_input, zooming_out))
        if(p_ship==0 || p_ship==1)
            p_ship += 2;
    if(PLAYERINPUT_PRESSED(p_input, accelerating)) {
        g->update = Game_updatePaletteSelection;
    }
    if(PLAYERINPUT_PRESSED(p_input, escaping)) {
        if(p > 0) {
            --p;
            g->update = Game_updateShipMenu;
        } else {
            Mix_PlayChannel(-1, g->snd_menu_transition, 0);
            g->update = Game_updateMainMenu;
            FadeTransition_fadeTo(&g->fade_transition, Game_renderMainMenu);
        }
    }
#undef p
#undef p_input
#undef p_ship
}
static void Game_updateMainMenu(Game *g) { 
    if(PLAYERINPUT_PRESSED(g->input.players[0], accelerating)) {
        Mix_PlayChannel(-1, g->snd_menu_transition, 0);
        g->ship_menu.player_index = 0;
        g->update = Game_updateShipMenu;
        FadeTransition_fadeTo(&g->fade_transition, Game_renderShipMenu);
    }

    if(PLAYERINPUT_PRESSED(g->input.players[0], left_tilting )
    || PLAYERINPUT_PRESSED(g->input.players[0], right_tilting)
    || PLAYERINPUT_PRESSED(g->input.players[0], zooming_in   )
    || PLAYERINPUT_PRESSED(g->input.players[0], zooming_out  ))
        g->player_count = (g->player_count<=1 ? 2 : 1);

    if(PLAYERINPUT_PRESSED(g->input.players[0], escaping))
        g->quit = true;
}

void Game_updateStartScreen(Game *g) {
    g->render = Game_renderMainMenu;
    g->update = Game_updateMainMenu;
    Mix_PlayMusic(g->main_music, -1);
    g->update(g);
}
void Game_update(Game *g) {
    g->update(g);
    /* L'ordre compte. S'assurer que g->update(g) soit d'abord. */
    g->fade_transition.update(g);
    g->clap_transition.update(&g->clap_transition, View_getOrthoTop(&g->menu_view));
    size_t i;
    for(i=0 ; i<MAX_PLAYERS ; ++i)
        g->input.players[i].old = g->input.players[i].now;
    if(g->update != Game_updateRace)
        return;
    switch(g->race.view_count) {
    case 1:
        View_mapPixelToCoords(g->race.views, &g->world_mouse_cursor, 
                             &g->input.old_mouse_pos);
        break;
    case 2:
        if(g->input.old_mouse_pos.x < g->window_size.x/2) {
            View_mapPixelToCoords(g->race.views, &g->world_mouse_cursor, 
                                 &g->input.old_mouse_pos);
            break;
        }
        View_mapPixelToCoords(g->race.views+1, &g->world_mouse_cursor, 
                             &g->input.old_mouse_pos);
        break;
    }
}

