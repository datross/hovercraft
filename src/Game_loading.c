#include <Utils.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <Game.h>
#include <SDL/SDL.h>
#include <GL/gl.h>

/* Ce que je fais avec le Parseur, c'est charger tout le fichier d'un coup dans
 * un buffer, et faire des sscanf() sur ce buffer. C'est juste plus pratique,
 * surtout pour effacer les commentaires en avance.
 * Le plus gros fichier de config que l'on a fait 1333 octets. */
#if BUFSIZ >= 4096
#define PARSER_MAX_FILE_SIZE BUFSIZ
#else
#define PARSER_MAX_FILE_SIZE 4096
#endif

#define PARSER_MAX_KEY_SIZE 128
#define PARSER_MAX_FILE_NAME_SIZE 512


typedef struct {
    char buf[PARSER_MAX_FILE_SIZE];
    char *cursor;
    size_t mapstart_index;
    SDL_Surface *teximg;
    SDL_Surface *palimg;
    GLuint tex[MAX_PALETTES];
} Parser;

bool Parser_init(Parser *p, const char *filename) {
    memset(p, 0, sizeof(*p));
    FILE *file = fopen(filename, "r");
    if(!file) {
        fprintf(stderr, "N'a pas pu ouvrir %s.\n", filename);
        return false;
    }
    if(fread(p->buf, 1, sizeof(p->buf), file) <= 0) {
        fprintf(stderr, "N'a pas pu lire %s.\n", filename);
        return false;
    }
    fclose(file);
    /* Retirer les commentaires */
    char *comm;
    for(comm=strchr(p->buf, '#') ; comm ; comm=strchr(comm, '#')) {
        char *nl = strchr(comm, '\n');
        if(!nl)
            nl = strchr(comm, '\0');
        memset(comm, ' ', nl-comm);
    }
    p->cursor = p->buf;
    return true;
}
void Parser_deinit(Parser *p) {
    if(p->teximg) SDL_FreeSurface(p->teximg);
    if(p->palimg) SDL_FreeSurface(p->palimg);
}

#define STREQ(s1,s2) (!strcmp(s1,s2))
#define STR(X) #X
#define XSTR(X) STR(X)

/* Le premier argument est le nombre d'items censés etre lus.  */
#define PARSER_SSCANF(cnt, parser, fmt, ...) \
    do { \
        int chars_read; \
        if(sscanf((parser)->cursor, fmt "%n", __VA_ARGS__, &chars_read) < cnt) \
            fprintf(stderr, "%s:%u: Erreur de parsing.\n", __FILE__, __LINE__); \
        (parser)->cursor += chars_read; \
    } while(0)


#define PARSER_BEGIN(parser) \
    char read_key[PARSER_MAX_KEY_SIZE]; \
    char read_filename[PARSER_MAX_FILE_NAME_SIZE]; \
    for(;;) { \
        int keylen; \
        if(sscanf((parser)->cursor, "%" XSTR(PARSER_MAX_KEY_SIZE) "s%n", read_key, &keylen) < 1) \
            break; \
        (parser)->cursor += keylen; \
        if(0);
#define PARSER_EXPECT_STRING(parser, key, ptr, len) \
        else if(STREQ(read_key, key)) { \
            PARSER_SSCANF(1, parser, "%" XSTR(len) "s", ptr); \
        }
#define PARSER_EXPECT_FLOAT(parser, key, ptr) \
        else if(STREQ(read_key, key)) { \
            PARSER_SSCANF(1, parser, "%f", ptr); \
        }
#define PARSER_END(parser) \
        else { \
            fprintf(stderr, "Clé ignorée : '%s'.\n", read_key); \
            char *nl = strchr((parser)->cursor, '\n'); \
            (parser)->cursor = nl ? nl+1 : strchr(((parser)->cursor), '\0'); \
        } \
    }
#define PARSER_EXPECT_VEC2(parser, key, ptr) \
        else if(STREQ(read_key, key)) { \
            PARSER_SSCANF(2, parser, "%f %f", &((ptr)->x), &((ptr)->y)); \
        }
#define PARSER_EXPECT_COLOR3(parser, key, ptr) \
        else if(STREQ(read_key, key)) { \
            PARSER_SSCANF(3, parser, "%f %f %f", &((ptr)->r), &((ptr)->g), &((ptr)->b)); \
        }
#define PARSER_EXPECT_TEXTURE(parser, key) \
        else if(STREQ(read_key, key)) { \
            PARSER_SSCANF(1, parser, "%" XSTR(PARSER_MAX_FILE_NAME_SIZE) "s", \
                          read_filename); \
            if((parser)->teximg) \
                SDL_FreeSurface((parser)->teximg); \
            (parser)->teximg = IMG_Load(read_filename); \
            if(!(parser)->teximg) { \
                fprintf(stderr, "Impossible de charger l'image '%s'\n", read_filename); \
                exit(EXIT_FAILURE); \
            } \
            if((parser)->teximg->format->BytesPerPixel != 4) { \
                fprintf(stderr, "Le format attendu de '%s' est RGBA.\n", read_filename); \
                exit(EXIT_FAILURE); \
            } \
            (parser)->tex[0] = Tex_loadFromSurface((parser)->teximg); \
        }
#define PARSER_EXPECT_TILEDQUAD(parser, key, ptr, quad_size_ptr) \
        else if(STREQ(read_key, key)) { \
            PARSER_SSCANF(3, parser, "%" XSTR(PARSER_MAX_FILE_NAME_SIZE) "s %f %f", \
                          read_filename, &(ptr)->tile_size.x, &(ptr)->tile_size.y); \
            (ptr)->tex_id = Tex_loadFromFile(read_filename);\
            if(!(ptr)->tex_id) \
                exit(EXIT_FAILURE); \
            (ptr)->quad_size = *(quad_size_ptr); \
        }

#define PARSER_EXPECT_PALETTE(parser, key) \
        else if(STREQ(read_key, key)) { \
            PARSER_SSCANF(1, parser, "%" XSTR(PARSER_MAX_FILE_NAME_SIZE) "s", \
                          read_filename); \
            if((parser)->palimg) \
                SDL_FreeSurface((parser)->palimg); \
            (parser)->palimg = IMG_Load(read_filename); \
            if(!(parser)->palimg) { \
                fprintf(stderr, "Impossible de charger l'image '%s'\n", read_filename); \
                exit(EXIT_FAILURE); \
            } \
            if((parser)->palimg->format->BytesPerPixel != 4) { \
                fprintf(stderr, "Le format attendu de '%s' est RGBA.\n", read_filename); \
                exit(EXIT_FAILURE); \
            } \
            /* Appliquer les palettes. */ \
            if(!(parser)->teximg) { \
                fprintf(stderr, "La palette a été spécifiée avant la texture.\n"); \
                exit(EXIT_FAILURE); \
            } \
            if((parser)->palimg->h < MAX_PALETTES) { \
                fprintf(stderr, "La palette doit au moins avoir %u lignes.\n", MAX_PALETTES); \
                exit(EXIT_FAILURE); \
            } \
            size_t p, y, x, c; \
            GLubyte *indices = malloc((parser)->teximg->w*(parser)->teximg->h); \
            if(!indices) { \
                fprintf(stderr, "N'a pas pu allouer %u indices.\n", (parser)->teximg->w*(parser)->teximg->h); \
                exit(EXIT_FAILURE); \
            } \
            for(y=0 ; y<(parser)->teximg->h ; ++y) \
                for(x=0 ; x<(parser)->teximg->w ; ++x) \
                    for(c=0 ; c<(parser)->palimg->w ; ++c) \
                        if (((*getRGBAptr((parser)->teximg,y,x))) == ((*getRGBAptr((parser)->palimg,0,c)))) \
                            indices[y*(parser)->teximg->w +x] = c+1; \
            for(p=1 ; p<MAX_PALETTES ; ++p) { \
                printf("Transfert de la palette %u...\n", (unsigned) p); \
                (parser)->tex[p] = Tex_loadFromIndices((parser)->palimg, p, (parser)->teximg->w, (parser)->teximg->h, indices); \
            } \
            free(indices); \
        }
#define PARSER_EXPECT_PALETTED_SPRITES(parser, key, ptr) \
        else if(STREQ(read_key, key)) { \
            Vec2u rect_pos, rect_size, img_size = { (parser)->teximg->w, (parser)->teximg->h }; \
            PARSER_SSCANF(4, parser, "%u %u %u %u", \
                          &rect_pos.x, &rect_pos.y, &rect_size.x, &rect_size.y); \
            size_t p; \
            for(p=0 ; p<MAX_PALETTES ; ++p) \
                Sprite_build((ptr)+p, (parser)->tex[p], rect_pos, rect_size, img_size); \
        }
#define PARSER_EXPECT_SPRITE(parser, key, ptr) \
        else if(STREQ(read_key, key)) { \
            Vec2u rect_pos, rect_size, img_size = { (parser)->teximg->w, (parser)->teximg->h }; \
            PARSER_SSCANF(4, parser, "%u %u %u %u", \
                          &rect_pos.x, &rect_pos.y, &rect_size.x, &rect_size.y); \
            Sprite_build(ptr, (parser)->tex[0], rect_pos, rect_size, img_size); \
        }
#define PARSER_EXPECT_MAPSTARTS(parser, key, ptr) \
        else if(STREQ(read_key, key)) { \
            MapStart *ms = (ptr)+(parser)->mapstart_index; \
            PARSER_SSCANF(3, parser, "%f %f %f", \
                          &ms->pos.x, &ms->pos.y, &ms->tilt); \
            ++((parser)->mapstart_index); \
        }
#define PARSER_EXPECT_CHECKPOINTS(parser, key, mapdata) \
        else if(STREQ(read_key, key)) { \
            if((mapdata)->checkpoint_count < MAX_CHECKPOINTS) { \
                Checkpoint *c = (mapdata)->checkpoints + (mapdata)->checkpoint_count; \
                PARSER_SSCANF(3, parser, "%f %f %f", \
                              &c->pos.x, &c->pos.y, &c->radius); \
                ++((mapdata)->checkpoint_count); \
            } else fprintf(stderr, "Un checkpoint a été ignoré. La limite est %u.\n", MAX_CHECKPOINTS); \
        }
#define PARSER_EXPECT_WALL(parser, key, mapdata) \
        else if(STREQ(read_key, key)) { \
            if(!current_wall) { \
                fprintf(stderr, "Erreur parsing murs : pas d'obstacle déclaré avant l'instanciation.\n"); \
                exit(EXIT_FAILURE); \
            } \
            Wall * buf = realloc(mapdata->walls, (++mapdata->wall_count) * sizeof(Wall)); \
            if(!buf) { \
                fprintf(stderr, "Erreur d'allocation mémoire des obstacles.\n"); \
                exit(EXIT_FAILURE); \
            } else { \
                mapdata->walls = buf; \
                Wall * w = &(mapdata->walls[mapdata->wall_count - 1]); \
                *w = *current_wall; \
                if(w->physic_obstacle.shape.type == POLYGON) { \
                    if(!(w->physic_obstacle.shape.shape.polygon.vertices = \
                            malloc(w->physic_obstacle.shape.shape.polygon.nb_vertices * sizeof(Vec2)))) { \
                        fprintf(stderr, "Erreur allocation mémoire pour les obstacles.\n"); \
                        exit(EXIT_FAILURE); \
                    } \
                    memcpy(w->physic_obstacle.shape.shape.polygon.vertices, \
                           current_wall->physic_obstacle.shape.shape.polygon.vertices, \
                           w->physic_obstacle.shape.shape.polygon.nb_vertices * sizeof(Vec2)); \
                } \
                PARSER_SSCANF(6, parser, "%f %f %f %f %f %f",  \
                    &(w->color.r), &(w->color.g), &(w->color.b), \
                    &(w->physic_obstacle.position.x), &(w->physic_obstacle.position.y), \
                    &(w->physic_obstacle.rotation)); \
            } \
        }
#define PARSER_EXPECT_OBSTACLE(parser, key) \
        else if(STREQ(read_key, key)) { \
            if(current_wall) ConvexShape_free_content(&(current_wall->physic_obstacle.shape)); \
            free(current_wall); \
            if(!(current_wall = calloc(1, sizeof(Wall)))) { \
                fprintf(stderr, "Erreur allocation structure tampon pour chargement des obstacles.\n"); \
                exit(EXIT_FAILURE); \
            } \
            char type[10]; \
            PARSER_SSCANF(1, parser, "%10s", type); \
            if(!strcmp(type, "polygon")) { \
                current_wall->physic_obstacle.shape.type = POLYGON; \
                unsigned nb_sommets = 0; \
                PARSER_SSCANF(1, parser, "%u", &nb_sommets); \
                current_wall->physic_obstacle.shape.shape.polygon.nb_vertices = nb_sommets; \
                if(!(current_wall->physic_obstacle.shape.shape.polygon.vertices = malloc(nb_sommets * sizeof(Vec2)))) { \
                    fprintf(stderr, "Erreur allocation mémoire pour les sommets des obstacles.\n"); \
                    exit(EXIT_FAILURE); \
                } \
                for(unsigned i = 0; i < nb_sommets; ++i) { \
                    PARSER_SSCANF(2, parser, "%f %f",  \
                                &(current_wall->physic_obstacle.shape.shape.polygon.vertices[i].x), \
                                &(current_wall->physic_obstacle.shape.shape.polygon.vertices[i].y)); \
                } \
            } else if(!strcmp(type, "circle")) { \
                current_wall->physic_obstacle.shape.type = CIRCLE; \
                PARSER_SSCANF(1, parser, "%f", &(current_wall->physic_obstacle.shape.shape.circle.radius)); \
                current_wall->physic_obstacle.shape.shape.circle.position = MakeVec2(0,0); \
            } else { \
                fprintf(stderr, "Erreur de type d'obstacle.\n"); \
                exit(EXIT_FAILURE); \
            } \
        }
#define PARSER_EXPECT_MUSIC(parser, key, mapdata) \
        else if(STREQ(read_key, key)) { \
            char music_path[PARSER_MAX_FILE_NAME_SIZE]; \
            PARSER_SSCANF(1, parser, "%s", music_path); \
            if(!(mapdata->music = Mix_LoadMUS(music_path))) { \
                fprintf(stderr, "Erreur chargement musique : %s\n", music_path); \
                exit(EXIT_FAILURE); \
            } \
        }

void Game_loadMenus(Game *g, const char *dirname) {
    Util_pushd(dirname);
    Parser the_parser;
    if(!Parser_init(&the_parser, "menus.txt"))
        exit(EXIT_FAILURE); /* Le message d'erreur a été affiché. */    
    PARSER_BEGIN(&the_parser)
    PARSER_EXPECT_TEXTURE(&the_parser, "texture")
    PARSER_EXPECT_SPRITE(&the_parser, "sky", &g->main_menu.sky)
    PARSER_EXPECT_SPRITE(&the_parser, "bg0",  g->main_menu.bg)
    PARSER_EXPECT_SPRITE(&the_parser, "bg1",  g->main_menu.bg+1)
    PARSER_EXPECT_SPRITE(&the_parser, "bg2",  g->main_menu.bg+2)
    PARSER_EXPECT_SPRITE(&the_parser, "bg3",  g->main_menu.bg+3)
    PARSER_EXPECT_SPRITE(&the_parser, "title", &g->main_menu.title)
    PARSER_EXPECT_SPRITE(&the_parser, "oneplayer", &g->main_menu.oneplayer)
    PARSER_EXPECT_SPRITE(&the_parser, "twoplayers", &g->main_menu.twoplayers)
    PARSER_EXPECT_SPRITE(&the_parser, "ship_menu_title0",  g->ship_menu.titles)
    PARSER_EXPECT_SPRITE(&the_parser, "ship_menu_title1",  g->ship_menu.titles+1)
    PARSER_EXPECT_SPRITE(&the_parser, "ship_cell", &g->ship_menu.ship_cell)
    PARSER_EXPECT_SPRITE(&the_parser, "map_left_arrow", &g->map_menu.left_arrow)
    PARSER_EXPECT_SPRITE(&the_parser, "clap_top", &g->clap_transition.top)
    PARSER_EXPECT_SPRITE(&the_parser, "clap_bottom", &g->clap_transition.bottom)
    PARSER_END(&the_parser)
    Parser_deinit(&the_parser);
    Util_popd();
}

static void loadMapData(MapData *m, const char *filename) {
    Parser the_parser;
    if(!Parser_init(&the_parser, filename))
        exit(EXIT_FAILURE); /* Le message d'erreur a été affiché. */
    Wall * current_wall = NULL;
    PARSER_BEGIN(&the_parser)
    PARSER_EXPECT_STRING(&the_parser, "name", m->name, MAP_NAME_LEN)
    PARSER_EXPECT_VEC2(&the_parser, "size", &m->size)
    PARSER_EXPECT_TEXTURE(&the_parser, "texture")
    PARSER_EXPECT_SPRITE(&the_parser, "artwork", &m->artwork)
    PARSER_EXPECT_SPRITE(&the_parser, "banner", &m->banner)
    PARSER_EXPECT_TILEDQUAD(&the_parser, "terrain_tile", &m->terrain, &m->size)
    PARSER_EXPECT_COLOR3(&the_parser, "color", &m->color)
    PARSER_EXPECT_FLOAT(&the_parser, "friction", &m->friction)
    PARSER_EXPECT_COLOR3(&the_parser, "checkpoint_color", &m->checkpoint_color)
    PARSER_EXPECT_COLOR3(&the_parser, "checkpoint_highlight", &m->checkpoint_highlight)
    PARSER_EXPECT_MAPSTARTS(&the_parser, "start", m->start)
    PARSER_EXPECT_CHECKPOINTS(&the_parser, "checkpoint", m)
    PARSER_EXPECT_OBSTACLE(&the_parser, "obstacle")
    PARSER_EXPECT_WALL(&the_parser, "wall", m)
    PARSER_EXPECT_MUSIC(&the_parser, "music", m)
    PARSER_END(&the_parser)
    Parser_deinit(&the_parser);
    if(current_wall) ConvexShape_free_content(&(current_wall->physic_obstacle.shape));
    free(current_wall);
}

static void loadShipData(ShipData *s, const char *filename) {
    float width;
    Parser the_parser;
    if(!Parser_init(&the_parser, filename))
        exit(EXIT_FAILURE); /* Le message d'erreur a été affiché. */
    PARSER_BEGIN(&the_parser)
    PARSER_EXPECT_FLOAT(&the_parser, "friction", &s->friction)
    PARSER_EXPECT_FLOAT(&the_parser, "tilt_step", &s->tilt_step)
    PARSER_EXPECT_FLOAT(&the_parser, "accel_multiplier", &s->accel_multiplier)
    PARSER_EXPECT_FLOAT(&the_parser, "max_speed", &s->max_speed)
    PARSER_EXPECT_FLOAT(&the_parser, "width", &width)
    PARSER_EXPECT_TEXTURE(&the_parser, "texture")
    PARSER_EXPECT_PALETTE(&the_parser, "palette")
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "banner",  s->banner)
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "side",    s->side)
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "icon",    s->icon)
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "artwork", s->artwork)
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "above0",  s->above[0])
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "above1",  s->above[1])
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "above2",  s->above[2])
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "above3",  s->above[3])
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "above4",  s->above[4])
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "above5",  s->above[5])
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "above6",  s->above[6])
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "above7",  s->above[7])
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "above8",  s->above[8])
    PARSER_EXPECT_PALETTED_SPRITES(&the_parser, "above9",  s->above[9])
    PARSER_EXPECT_VEC2(&the_parser, "reactor_position", &(s->reactor_position))
    PARSER_END(&the_parser)
    Parser_deinit(&the_parser);
    size_t i, p;
    for(i=0 ; i<10 ; ++i)
        for(p=0 ; p<MAX_PALETTES ; ++p)
            Sprite_resizeToWidth(&s->above[i][p], width);
    s->reactor_distance_to_center = sqrt(Scal2(s->reactor_position,
                                            s->reactor_position));
}

void Game_loadMaps(Game *g, const char *dirname) {
    DIR *dir = opendir(dirname);
    if(!dir) {
        fprintf(stderr, "N'a pas pu ouvrir %s.\n", dirname);
        exit(EXIT_FAILURE);
    }
    Util_pushd(dirname);
    struct dirent *entry;
    for(g->map_data_count=0 ; g->map_data_count<MAX_MAPS ; ) {
        entry = readdir(dir);
        if(!entry) {
            if(!g->map_data_count) {
                fprintf(stderr, "Aucune map n'a été trouvée.\n");
                exit(EXIT_FAILURE);
            }
            break;
        }
        const char *extension = strchr(entry->d_name, '.');
        if(!extension || strcasecmp(extension+1, "txt"))
            continue;
        printf("Chargement de '%s'...\n", entry->d_name);
        loadMapData(g->map_data+g->map_data_count, entry->d_name);
        ++(g->map_data_count);
    }
    closedir(dir);
    Util_popd();
}

void Game_loadShips(Game *g, const char *dirname) {
    DIR *dir = opendir(dirname);
    if(!dir) {
        fprintf(stderr, "N'a pas pu ouvrir %s.\n", dirname);
        exit(EXIT_FAILURE);
    }
    Util_pushd(dirname);
    size_t i=0;
    struct dirent *entry;
    for(i=0 ; i<MAX_SHIPS ; ) {
        entry = readdir(dir);
        if(!entry) {
            fprintf(stderr, "Vaisseaux manquants. %u attendus.\n", MAX_SHIPS);
            exit(EXIT_FAILURE);
        }
        const char *extension = strchr(entry->d_name, '.');
        if(!extension || strcasecmp(extension+1, "txt"))
            continue;
        printf("Chargement de '%s'...\n", entry->d_name);
        loadShipData(g->ship_data+i, entry->d_name);
        ++i;
    }
    closedir(dir);
    Util_popd();
}

void Game_loadSounds(Game *g, const char *dirname_music, const char *dirname_snd) {
    Util_pushd(dirname_music);
    g->main_music = Mix_LoadMUS("main_menu_music.mp3");
    Util_popd();
    Util_pushd(dirname_snd);
#define LOAD_SND(ptr, file) \
    if(!(ptr = Mix_LoadWAV(file))) { \
        fprintf(stderr, "Erreur chargement son : %s.\n", file); \
        exit(EXIT_FAILURE); \
    } 
    LOAD_SND(g->snd_menu_transition, "menu_transition.wav")
    LOAD_SND(g->snd_clap_close     , "clap_close.wav")
    Util_popd();
}
