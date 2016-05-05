#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <Vec.h>
#include <time.h>

#define max(a,b) ((a)>(b) ? (a) : (b))
#define degf(a) ((a)*180.f/M_PI)
#define radf(a) ((a)*M_PI/180.f)

typedef struct {
    float r, g, b;
} Color3;

static inline void Random_init() {
    srand(time(NULL));
}

static inline float Rand_f_0_1() { /* entre 0 et 1 */
    return (float)rand() / (float)RAND_MAX;
}

static inline float Rand_f_1() { /* entre -1 et 1 */
    return -1 + 2*(float)rand() / (float)RAND_MAX;
}

static inline void renderSquare(bool full) {
    glBegin(full ? GL_QUADS : GL_LINE_LOOP);
    glVertex2f(-.5, -.5);
    glVertex2f( .5, -.5);
    glVertex2f( .5,  .5);
    glVertex2f(-.5,  .5);
    glEnd();
}

static inline void renderIsocelesTriangle(bool full) {
    glBegin(full ? GL_TRIANGLES : GL_LINE_LOOP);
    glVertex2f(  0,  .5);
    glVertex2f(-.5, -.5);
    glVertex2f( .5, -.5);
    glEnd();
}

static inline void renderAxes(void) {
    GLfloat col[4];
    glGetFloatv(GL_CURRENT_COLOR, col);

    glColor3f(0,1,0);
    glBegin(GL_LINES);
    glVertex2f(0, 0);
    glVertex2f(0, 1);
    glEnd();

    glColor3f(1,0,0);
    glBegin(GL_LINES);
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glEnd();

    glColor3f(0,0,1);
    glBegin(GL_LINES);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1);
    glEnd();

    glColor4f(col[0], col[1], col[2], col[3]);
}

#define renderDisk(seg) renderCircle(seg, true)
static inline void renderCircle(unsigned seg, bool full) {
    if(seg<3)
        return;
    glBegin(full ? GL_POLYGON : GL_LINE_LOOP);
    unsigned i;
    for(i=1 ; i<=seg ; ++i) {
        float theta = 2*M_PI*i/(float)seg;
        glVertex2f(cosf(theta), sinf(theta));
    }
    glEnd();
}

typedef struct {
    GLuint tex_id;
    Vec2 tile_size;
    Vec2 quad_size;
} TiledQuad;

static inline void TiledQuad_render(const TiledQuad *tq) {
    Vec2 texcoords = {
        tq->quad_size.x/tq->tile_size.x,
        tq->quad_size.y/tq->tile_size.y
    };
    glColor4f(1.f, 1.f, 1.f, 1.f);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tq->tex_id);
    glPushMatrix();
    {
        glScalef(tq->quad_size.x, tq->quad_size.y, 1.f);
        glBegin(GL_QUADS);
        glTexCoord2f(0, texcoords.y);
        glVertex2f(-.5f, -.5f);
        glTexCoord2f(texcoords.x, texcoords.y);
        glVertex2f(.5f, -.5f);
        glTexCoord2f(texcoords.y, 0);
        glVertex2f(.5f, .5f);
        glTexCoord2f(0, 0);
        glVertex2f(-.5f, .5f);
        glEnd();
    }
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

static inline GLuint Tex_loadFromSurface(SDL_Surface *image) {
    GLuint tex = 0;
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &tex);
    assert(tex != 0 && "Il faut d'abord initialiser OpenGL.");
    glBindTexture(GL_TEXTURE_2D, tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    GLenum format;
    switch(image->format->BytesPerPixel) {
        case 1: format = GL_RED; break;
        case 3: format = GL_RGB; break;
        case 4: format = GL_RGBA; break;
        default: fprintf(stderr, "Format des pixels de l'image "
                         "non pris en charge\n");
            return 0;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 
                 0, format, GL_UNSIGNED_BYTE, image->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    return tex;
}
static inline GLuint Tex_loadFromFile(const char *filename) {
    SDL_Surface* image = IMG_Load(filename);
    if(!image) {
        fprintf(stderr, "Impossible de charger l'image %s\n", filename);
        return 0;
    }
    GLuint tex = Tex_loadFromSurface(image);
    SDL_FreeSurface(image);
    return tex;
}


static inline uint32_t* getRGBAptr(SDL_Surface *img, size_t y, size_t x) {
    return ((uint32_t*) img->pixels) + y*img->w + x;
}
#define DEFINE_GETCHANNEL(C) \
static inline uint8_t get##C(SDL_Surface *img, size_t y, size_t x) { \
    uint32_t tmp = *getRGBAptr(img, y, x); \
    tmp  &= img->format->C##mask; \
    tmp >>= img->format->C##shift; \
    tmp <<= img->format->C##loss; \
    return tmp; \
}
DEFINE_GETCHANNEL(R)
DEFINE_GETCHANNEL(G)
DEFINE_GETCHANNEL(B)
DEFINE_GETCHANNEL(A)
#undef DEFINE_GETCHANNEL

static inline GLuint Tex_loadFromIndices(SDL_Surface *pal, size_t palindex, size_t width, size_t height, const GLubyte *indices) {
    /* D'après la spec OpenGL */
#define MAX_PAL_COLORS 32
    assert(pal->w+1 <= MAX_PAL_COLORS);
    glPixelTransferi(GL_MAP_COLOR,GL_TRUE);

    GLfloat rmap[MAX_PAL_COLORS];
    GLfloat gmap[MAX_PAL_COLORS];
    GLfloat bmap[MAX_PAL_COLORS];
    GLfloat amap[MAX_PAL_COLORS];
    memset(rmap, 0, sizeof(rmap));
    memset(gmap, 0, sizeof(gmap));
    memset(bmap, 0, sizeof(bmap));
    memset(amap, 0, sizeof(amap));
    size_t c;
    for(c=0 ; c<pal->w ; ++c) {
        rmap[c+1] = getR(pal, palindex, c)/255.f;
        gmap[c+1] = getG(pal, palindex, c)/255.f;
        bmap[c+1] = getB(pal, palindex, c)/255.f;
        amap[c+1] = getA(pal, palindex, c)/255.f;
    }

    glPixelMapfv(GL_PIXEL_MAP_I_TO_R, MAX_PAL_COLORS, rmap);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_G, MAX_PAL_COLORS, gmap);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_B, MAX_PAL_COLORS, bmap);
    glPixelMapfv(GL_PIXEL_MAP_I_TO_A, MAX_PAL_COLORS, amap);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 
                 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, indices);
    glPixelTransferi(GL_MAP_COLOR,GL_FALSE);
    return tex;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static int old_cwd = -1;

static inline void Util_pushd(const char *dirname) {
    old_cwd = open(".", O_RDONLY | O_DIRECTORY, 0666);
    if(chdir(dirname) != -1)
        return;
    perror("chdir");
    exit(EXIT_FAILURE);
}
static inline void Util_popd(void) {
    if(old_cwd == -1) 
        return;
    if(fchdir(old_cwd)==-1)
        perror("fchdir()");
    close(old_cwd);
    old_cwd = -1;
}

/* La portion de code suivante vient de F.A.T.E. 
 * Le but est d'avoir une fonction Util_getExecutablePath() afin
 * d'en faire le répertoire courant pour trouver les ressources. */

#if !(BSD_SOURCE || _XOPEN_SOURCE >= 500  \
 || _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED || _POSIX_C_SOURCE >= 200112L)
#ifdef _GNU_SOURCE
#include <sys/syscall.h>
static inline ssize_t readlink(const char *path, char *buf, size_t bufsiz) {
    return syscall(SYS_readlink, path, buf, bufsiz);
}
static inline int lstat(const char *path, struct stat *buf) {
    return syscall(SYS_lstat, path, buf);
}
#else
#error Syscalls lstat() and readlink() are not available. Please update (g)libc or define _GNU_SOURCE.
#endif
#endif
#if __GLIBC__ > 2 || (__GLIBC__==2 && __GLIBC_MINOR__>=16)
#include <sys/auxv.h>
#endif
static inline char *Util_getExecutablePath(void) {
    struct stat st;
    char *str2, *str;
#if __GLIBC__ > 2 || (__GLIBC__==2 && __GLIBC_MINOR__>=16)
    str = (char*)getauxval(AT_EXECFN);
    if(str)
        return realpath(str, NULL);
#endif
    if(lstat("/proc/self/exe", &st) == 0) {
        str = malloc(st.st_size+1);
        if(readlink("/proc/self/exe", str, st.st_size) > 0) {
            str[st.st_size] = '\0';
            str2 = realpath(str, NULL);
            free(str);
            return str2;
        }
        free(str);
    }
    return NULL;
}


#endif /* UTILS_H */
