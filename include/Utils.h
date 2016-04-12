#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>

#define max(a,b) ((a)>(b) ? (a) : (b))
#define degf(a) ((a)*180.f/M_PI)
#define radf(a) ((a)*M_PI/180.f)

typedef struct {
    float r, g, b;
} Color3;

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

static inline GLuint Tex_loadFromFile(const char *filename) {
    SDL_Surface* image = IMG_Load(filename);
    if(!image) {
        fprintf(stderr, "Impossible de charger l'image %s\n", filename);
        return 0;
    }

    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    GLenum format;
    switch(image->format->BytesPerPixel) {
        case 1: format = GL_RED; break;
        case 3: format = GL_RGB; break;
        case 4: format = GL_RGBA; break;
        default: fprintf(stderr, "Format des pixels de l'image "
                                 "%s non pris en charge\n", filename);
            return 0;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 
                 0, format, GL_UNSIGNED_BYTE, image->pixels);
    
    SDL_FreeSurface(image);

    return textureId;
}



#endif /* UTILS_H */
