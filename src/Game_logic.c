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
        Vec2 diff = {s->guides[i].pos.x - s->physic_solid.position.x, 
                     s->guides[i].pos.y - s->physic_solid.position.y};
        s->guides[i].theta = degf(atan2f(diff.y, diff.x))-90.f;
        s->guides[i].distance = sqrtf(diff.x*diff.x + diff.y*diff.y);
        /*
        printf("Distance from guides[%zu] : %u units.\n", 
                i, (uint32_t)s->guides[i].distance);
        */
    }
}
static void Game_updateMainMenu(Game *g); /* prototype pour Race(). */

static void Game_updateRace(Game *g) { 

    uint32_t new_ms = SDL_GetTicks();
    g->race_state.time_ms += new_ms - g->race_state.step_ms;
    g->race_state.step_ms = new_ms;

    size_t i;
    for(i=0 ; i<g->ship_count ; ++i) {
        Ship *s = g->ships+i;

        if(g->input.players[i].accelerating) {
            s->main_translate_force.force.x = s->accel_multiplier*cosf(s->physic_solid.rotation);
            s->main_translate_force.force.y = s->accel_multiplier*sinf(s->physic_solid.rotation);
            World_addForce(&(g->world), &(s->main_translate_force));
        } 

        if(g->input.players[i].left_tilting) {
			s->main_rotate_force.force.x = s->tilt_step;
			World_addForce(&(g->world), &(s->main_rotate_force));
        }
        if(g->input.players[i].right_tilting) {
			s->main_rotate_force.force.x = - s->tilt_step;
			World_addForce(&(g->world), &(s->main_rotate_force));
        }


        /* La friction joue beaucoup sur la vitesse maximale
         * factuellement atteignable des véhicules. */
        
        s->main_translation_friction.force = MulVec2(s->physic_solid.speed, -s->friction);
        s->main_rotation_friction.force.x = -s->friction * s->physic_solid.rotation_speed;
 
        World_addForce(&(g->world), &(s->main_translation_friction));
        World_addForce(&(g->world), &(s->main_rotation_friction));

        const float dist = sqrtf(s->physic_solid.speed.x*s->physic_solid.speed.x + s->physic_solid.speed.y*s->physic_solid.speed.y);
        const float speed = 1000.f*dist/g->tickrate;
        if(speed > s->max_speed) {
            printf("Reaching max speed.\n");
            /* FIXME C'est une approximation qui est précise
             * avec 'max_speed' élevée, mais pas tant que ça
             * quand elle est faible. */
            /*s->vel.x *= s->max_speed/speed;
            s->vel.y *= s->max_speed/speed;*/
        }

        {
            const Checkpoint *c = g->map.checkpoints 
                                  + s->next_checkpoint_index;
            Vec2 diff = { c->pos.x - s->physic_solid.position.x, c->pos.y - s->physic_solid.position.y };
            float dist = sqrtf(diff.x*diff.x + diff.y*diff.y);
            if(dist <= c->radius)
                ++(s->next_checkpoint_index);
            if(s->next_checkpoint_index >= g->map.checkpoint_count) {
                printf("It's over ! Player %zu won.\n", i+1);
                printf("elapsed time : %d ms\n", g->race_state.time_ms);
                /* Game_quit(g); */
                s->next_checkpoint_index = 0; /* Bien sûr on retire ça après. */
            } else {
                s->guides[0].pos.x = c->pos.x;
                s->guides[0].pos.y = c->pos.y;
            }
       }

        const size_t j = (i+1)%(g->ship_count);
        s->guides[1].pos.x = g->ships[j].physic_solid.position.x;
        s->guides[1].pos.y = g->ships[j].physic_solid.position.y;
        Ship_refreshGuides(s);

        /*
        printf("pos:(%f, %f); tilt: %f deg; speed: %u units/s\n", 
               s->pos.x, s->pos.y, degf(s->tilt), (uint32_t)speed);
        */

        if(i>=g->view_count)
            break;
        g->views[i].center.x = s->physic_solid.position.x;
        g->views[i].center.y = s->physic_solid.position.y;
        //g->views[i].tilt = atan(s->physic_solid.speed.y / s->physic_solid.speed.x);
        g->views[i].tilt = s->physic_solid.rotation;
        if(g->input.players[i].zooming_in && g->views[i].zoom < 6.f)
            g->views[i].zoom *= 1.1f;
        if(g->input.players[i].zooming_out && g->views[i].zoom > 0.02f)
            g->views[i].zoom *= .9f;
        /* if(i==0)printf("zoom : %f\n", g->views[i].zoom); */
    }
	Process_physics(&(g->world), g->tickrate);
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
    g->race_state.time_ms += new_ms - g->race_state.step_ms;
    g->race_state.step_ms = new_ms;
    if(1-g->race_state.time_ms > 0) {
        //printf("%d...\n", 1-g->race_state.time_ms/1000);
        return;
    }
    puts("Go!!!");
    g->update = Game_updateRace;
    g->update(g);
}
static void Game_updatePreCountdown(Game *g) {
    size_t i;
    for(i=0 ; i<g->view_count ; ++i) {
        g->views[i].center.x = g->ships[i].physic_solid.position.x;
        g->views[i].center.y = g->ships[i].physic_solid.position.y;
        g->views[i].tilt = g->ships[i].physic_solid.rotation;
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
        g->ships[i].guides[1].pos.x = g->ships[j].physic_solid.position.x;
        g->ships[i].guides[1].pos.y = g->ships[j].physic_solid.position.y;
        g->ships[i].guides[1].scale.x = .4f;
        g->ships[i].guides[1].scale.y = .4f;
        g->ships[i].guides[1].r = g->ships[j].r;
        g->ships[i].guides[1].g = g->ships[j].g;
        g->ships[i].guides[1].b = g->ships[j].b;
        g->ships[i].guide_count = 2;
        Ship_refreshGuides(g->ships+i);
    }
    g->race_state.time_ms = -3000;
    g->race_state.step_ms = SDL_GetTicks();

    g->update = Game_updateCountdown;
    g->update(g);
}
static void Game_updateMapSelection(Game *g) {
/*
    static unsigned selected_map_index = 0;
#define PRESSED(ipt) (g->input.players[current_player_index].ipt \
                    < g->input.players_old[current_player_index].ipt)
    if(PRESSED(left_tilting) || PRESSED(zooming_in)) {
        if(selected_map_index>0)
            --selected_map_index;
        else selected_map_index = g->map_count-1;
    }
    if(PRESSED(right_tilting) || PRESSED(zooming_out)) {
        ++selected_map_index;
        selected_map_index %= g->map_count;
    }
#undef PRESSED

    Puis afficher la bonne map...
*/
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

	g->world.solids       = NULL;
	g->world.forces_head  = NULL;
	g->world.forces_tail  = NULL;

	unsigned i = 0;
	for(; i < g->ship_count; ++i)
    	World_addSolid(&(g->world), &(g->ships[i].physic_solid)); 
	
    g->update = Game_updatePreCountdown;
    g->update(g);
}
static void Game_updateShipSelection(Game *g) {
    static unsigned current_player_index = 0;
    static unsigned selected_ship_index = 0;
#define PRESSED(ipt) (g->input.players[current_player_index].ipt \
                    < g->input.players_old[current_player_index].ipt)
    if(PRESSED(left_tilting))
        if(selected_ship_index==1 || selected_ship_index==3)
            --selected_ship_index;
    if(PRESSED(right_tilting))
        if(selected_ship_index==0 || selected_ship_index==2)
            ++selected_ship_index;
    if(PRESSED(zooming_in))
        if(selected_ship_index==2 || selected_ship_index==3)
            selected_ship_index -= 2;
    if(PRESSED(zooming_out))
        if(selected_ship_index==0 || selected_ship_index==1)
            selected_ship_index += 2;
#undef PRESSED

    /* Changer la position du curseur en fonction de selected_ship_index.
     * Changer sa couleur en fonction de current_player_index ? */

    memset(g->ships + current_player_index, 0, sizeof(Ship));
    Ship_init(g->ships + current_player_index);

    if(current_player_index < g->ship_count-1) {
        ++current_player_index;
        g->update = Game_updateShipSelection;
    } else {
        current_player_index = 0;
        g->update = Game_updateMapSelection;
    }
}
static void Game_updateMainMenu(Game *g) { 
    g->update = Game_updateShipSelection; /* A retirer plus tard. */
/*
    if(g->input.players[0].accelerating < g->input.players_old[0].accelerating)
        g->update = Game_updateShipSelection;

    if(g->input.players[0].left_tilting  < g->input.players_old[0].left_tilting
    || g->input.players[0].right_tilting < g->input.players_old[0].right_tilting
    || g->input.players[0].zooming_in    < g->input.players_old[0].zooming_in
    || g->input.players[0].zooming_out   < g->input.players_old[0].zooming_out)
        g->ship_count = (g->ship_count==1 ? 2 : 1);
*/
    /* Changer la position du curseur en fonction de g->ship_count. */
}
void Game_updateStartScreen(Game *g) {
    g->ship_count = 1;
    g->update = Game_updateMainMenu;
    g->update(g);
}
void Game_update(Game *g) {
    g->update(g);
    size_t i;
    for(i=0 ; i<MAX_PLAYERS ; ++i)
        g->input.players[i].old = g->input.players[i].now;
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

