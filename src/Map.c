#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <GL/gl.h>
#include <Map.h>
#include <Utils.h>

#define STREQ(s1,s2) (!strncmp(s1,s2,strlen(s2)))

void MapPreview_loadFromFile(MapPreview *m, FILE *file) {
    char buf[BUFSIZ];
    char *str;
    for(;;) {
        if(!fgets(buf, sizeof(buf), stdin))
            break;
        if(buf[0] == '#')
            continue;
        if(STREQ(buf, "name"))
            strncpy(m->name, buf+5, MAP_NAME_LEN);
        else if(STREQ(buf, "artwork")) {
            str = buf + strlen("artwork") + 1;
            *strchr(str, ' ') = '\0';
            m->texture_id = Tex_loadFromFile(str);
            assert(m->texture_id);
            str += strlen(str)+1;
            /* TODO */
            //sscanf(str, "%u %u %u %u", );
        }
    }
}

void Map_loadFromFile(Map *m, FILE *file) {
    char buf[BUFSIZ];
    char *str;
    for(;;) {
        if(!fgets(buf, sizeof(buf), stdin))
            break;
        if(buf[0] == '#')
            continue;
        if(STREQ(buf, "name")) {}
        else if(STREQ(buf, "size")) {}
        else if(STREQ(buf, "terrain")) {}
        else if(STREQ(buf, "color")) {}
        else if(STREQ(buf, "friction")) {}
        else if(STREQ(buf, "checkpoint_color")) {}
        else if(STREQ(buf, "checkpoint_color_highlight")) {}
        else if(STREQ(buf, "start")) {}
        else if(STREQ(buf, "checkpoint")) {}
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
