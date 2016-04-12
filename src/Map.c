#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <GL/gl.h>
#include <Map.h>
#include <Utils.h>

#define STREQ(s1,s2) (!strncmp(s1,s2,strlen(s2)))
#define STR(X) #X
#define XSTR(X) STR(X)

void MapPreview_loadFromFile(MapPreview *m, FILE *file) {
    char buf[BUFSIZ];
    char str[MAP_NAME_LEN];
    for(;;) {
        if(!fgets(buf, sizeof(buf), stdin))
            break;
        if(buf[0] == '#')
            continue;
        if(STREQ(buf, "name"))
            scanf("%*s %"XSTR(MAP_NAME_LEN)"s", m->name);
        else if(STREQ(buf, "artwork")) {
            unsigned x, y, w, h;
			scanf("%*s %"XSTR(MAP_NAME_LEN)"s %u %u %u %u", str, &x, &y, &w, &h);
			m->texture_id = Tex_loadFromFile(str);
            assert(m->texture_id);
            // TODO set texture clip


            break;
        }
    }
}

void Map_loadFromFile(Map *m, FILE *file) {
    char buf[BUFSIZ];
    char str[MAP_NAME_LEN];
    char num_player = 0;
    m->checkpoint_count = 0;

    for(;;) {
        if(!fgets(buf, sizeof(buf), stdin))
            break;
        if(buf[0] == '#')
            continue;
            
             if(STREQ(buf, "name")) scanf("%*s %"XSTR(MAP_NAME_LEN)"s", m->name);
        else if(STREQ(buf, "size")) scanf("%*s %f %f", &(m->size.x), &(m->size.y));
        else if(STREQ(buf, "terrain")) {
			unsigned x, y, w, h;
			scanf("%*s %"XSTR(MAP_NAME_LEN)"s %u %u %u %u", str, &x, &y, &w, &h);
			m->texture_terrain_id = Tex_loadFromFile(str);
            assert(m->texture_terrain_id);
            // TODO set texture clip
        }
        else if(STREQ(buf, "color")) scanf("%*s %f %f %f", &(m->color.r), &(m->color.g), &(m->color.b));
        else if(STREQ(buf, "friction")) scanf("%*s %f", &(m->friction));
        else if(STREQ(buf, "checkpoint_color")) scanf("%*s %f %f %f", &(m->color_checkpoint.r),
                                                      &(m->color_checkpoint.g), &(m->color_checkpoint.b));
        else if(STREQ(buf, "checkpoint_color_highlight")) scanf("%*s %f %f %f",
                                                                &(m->color_checkpoint_highlight.r),
                                                                &(m->color_checkpoint_highlight.g),
                                                                &(m->color_checkpoint_highlight.b));
        else if(STREQ(buf, "start")) {
            if(num_player == 1) break;
			// Pourquoi 4 nombres pour chaque joueur ?

			++num_player;
        }
        else if(STREQ(buf, "checkpoint")) {
			if(m->checkpoint_count >= MAX_CHECKPOINT_COUNT) break;

			scanf("%*s %f %f %f", &(m->checkpoints[m->checkpoint_count].pos.x),
								  &(m->checkpoints[m->checkpoint_count].pos.y),
								  &(m->checkpoints[m->checkpoint_count].radius));

			++(m->checkpoint_count);								  
        }
    }
}

static void Map_renderCheckpoint(const Checkpoint *c) {
    glPushMatrix();
    {
        glTranslatef(c->pos.x, c->pos.y, 0.f);
        glScalef(c->radius, c->radius, 1.f);
        renderCircle(64, false);
    }
    glPopMatrix();
}

void Map_renderCheckpoints(const Map *m, size_t next_checkpoint_index) {
    size_t i;
    glColor3f(0.f, 1.f, 0.6f);
    for(i=0 ; i<next_checkpoint_index ; ++i)
        Map_renderCheckpoint(m->checkpoints+i);
    glColor3f(1.f, 0.f, 0.f);
    for(    ; i<m->checkpoint_count ; ++i)
        Map_renderCheckpoint(m->checkpoints+i);
}
void Map_render(const Map *m) {
    renderAxes();

    glColor3f(0.f, .4f, .7f);
    glPushMatrix();
    {
        glScalef(2*m->size.x, 2*m->size.y, 1);
        renderSquare(true);
    }
    glPopMatrix();
}
