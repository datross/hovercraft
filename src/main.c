#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <Game.h>

static const Uint32 FRAMERATE_MILLISECONDS = 1000 / 60;

int main(int argc, char* argv[]) {

    if(SDL_Init(SDL_INIT_VIDEO) == -1) {
        fprintf(stderr, "Impossible d'initialiser la SDL. Fin du programme.\n");
        return EXIT_FAILURE;
    }

    Game game;
    Game_init(&game);

    game.win_w = 800;
    game.win_h = 600;
    game.bits_per_pixel = 32;
    game.ortho2d.left   = -1.f;
    game.ortho2d.right  =  1.f;
    game.ortho2d.bottom = -1.f;
    game.ortho2d.top    =  1.f;

    Game_reshape(&game);

    SDL_WM_SetCaption("Hovercraft", NULL);
    
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
    SDL_Quit();
    
    return EXIT_SUCCESS;
}
