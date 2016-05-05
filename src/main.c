#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <Game.h>

static const Uint32 FRAMERATE_MILLISECONDS = 1000 / 60;

int main(int argc, char* argv[]) {

    if(SDL_Init(SDL_INIT_VIDEO) == -1) {
        fprintf(stderr, "Impossible d'initialiser la SDL. Fin du programme.\n");
        return EXIT_FAILURE;
    }

    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    if((IMG_Init(flags) & flags) != flags) {
        printf("IMG_Init: Failed to init required jpg and png support!\n");
        printf("IMG_Init: %s\n", IMG_GetError());
        return EXIT_FAILURE;
    }
    
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1) 
    {
        printf("%s", Mix_GetError());
        exit(EXIT_FAILURE);
    }
    
    Random_init();

    Game game;
    Game_init(&game);
    game.tickrate = FRAMERATE_MILLISECONDS;

    SDL_WM_SetCaption("Hovercraft", NULL);
    
    glClearColor(0.f, 1.f, 0.f, 1.f);
    for(game.quit=false ; !game.quit ; ) {
        Uint32 startTime = SDL_GetTicks();
                
        SDL_Event e;
        while(SDL_PollEvent(&e))
            Game_handleEvent(&game, &e);

        Game_update(&game);

        glClear(GL_COLOR_BUFFER_BIT);
        Game_render(&game);        
        SDL_GL_SwapBuffers();

        Uint32 elapsedTime = SDL_GetTicks() - startTime;
        if(elapsedTime < FRAMERATE_MILLISECONDS)
            SDL_Delay(FRAMERATE_MILLISECONDS - elapsedTime);
    }
    
    Game_deinit(&game);
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
    
    return EXIT_SUCCESS;
}
