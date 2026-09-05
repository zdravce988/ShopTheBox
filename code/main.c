#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "game.h"
#include "game.c"

#define VIRTUAL_WINDOW_WIDTH 1600
#define VIRTUAL_WINDOW_HEIGHT 720
#define global_variable static

#define Assert(cond) do { if(!(cond)) *(int*)0 = 1; } while(0)

global_variable SDL_FRect textureCoords[] = {
    {0, 570, 140, 190},		//card 1
    {140, 380, 140, 190},	//card 2
    {140, 190, 140, 190},	//card 3
    {140, 0, 140, 190},		//card 4
    {0, 1710, 140, 190},	//card 5
    {0, 1520, 140, 190},	//card 6
    {0, 1330, 140, 190},	//card 7
    {0, 1140, 140, 190},	//card 8
    {0, 950, 140, 190},		//card 9
    {0, 760, 140, 190},		//card 10
    {1024, 0, 140, 190},	//back of the card
    {64, 2048, 64, 64},		//dice1
    {128, 2048, 64, 64},	//dice2
    {64, 2112, 64, 64},		//dice3
    {128, 2112, 64, 64},	//dice4
    {0, 2112, 64, 64},		//dice5
    {0, 2048, 64, 64},		//dice6
    {2048, 0, 192, 64},		//red button
    {2048, 64, 192, 64},	//red pressed
    {0, 4096, 384, 128},	//green button
    {0, 4224, 384, 128},	//green pressed 
    {0, 4352, 384, 128},	//green not allowed
};

int main()
{
    SDL_srand(0);
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
	return 0;
    }

    SDL_Window *window = SDL_CreateWindow("SHOP THE BOX", VIRTUAL_WINDOW_WIDTH, VIRTUAL_WINDOW_HEIGHT, 0);

    if(!window)
    {
	SDL_Quit();
	return 0;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, 0);

    if(!renderer)
    {
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
    }

    if(TTF_Init() < 0)
    {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
    }

    TTF_Font *font = TTF_OpenFont("../assets/Kenney Blocks.ttf", 24); 
    if(!font)
    {
	return -1;
    }

    int width, height, numOfChannels;

    unsigned char *data = stbi_load("../assets/atlas.png", &width, &height, &numOfChannels, 4);

    if(!data)
    {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
    }

    SDL_SetRenderLogicalPresentation(renderer, VIRTUAL_WINDOW_WIDTH, VIRTUAL_WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetWindowFullscreen(window, 1);


    SDL_Surface *atlasSurface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);

    atlasSurface->pixels = data; 

    SDL_Texture *atlas = SDL_CreateTextureFromSurface(renderer, atlasSurface);
    if(!atlas)
    {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
    }

    SDL_SetTextureBlendMode(atlas, SDL_BLENDMODE_BLEND);

    stbi_image_free(data);

    SDL_SetRenderVSync(renderer, 1);
    gameState.camX = 0;
    gameState.camY = 0;

    gameState.commandBuffer.data = malloc(4 * 1024 * 1024);
    gameState.commandBuffer.base = gameState.commandBuffer.data;
    gameState.commandBuffer.end = gameState.commandBuffer.base;
    gameState.GameRand = SDL_rand;
    gameState.bestScore = 55;
    gameState.scoreText = (char*)malloc(100);
    gameState.bestScoreText = (char*)malloc(100);
    gameState.gameInitialized = 0;

    uint64_t lastTick = 0;

    for(;;)
    {
	SDL_Event event;
	if(SDL_PollEvent(&event))
	{
	    if(event.type == SDL_EVENT_QUIT)
	    {
		break;
	    }
	    else if(event.type == SDL_EVENT_MOUSE_BUTTON_UP)
	    {
		SDL_ConvertEventToRenderCoordinates(renderer, &event);
		if(event.button.button == SDL_BUTTON_LEFT)
		{
		    gameState.gameInput.mouseX = event.button.x;
		    gameState.gameInput.mouseY = event.button.y;
		    gameState.gameInput.mouseUp = 1;
		    gameState.gameInput.mouseDown = 0;
		}
	    }
	    else if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
	    {
		SDL_ConvertEventToRenderCoordinates(renderer, &event);
		if(event.button.button == SDL_BUTTON_LEFT)
		{
		    gameState.gameInput.mouseX = event.button.x;
		    gameState.gameInput.mouseY = event.button.y;
		    gameState.gameInput.mouseDown = 1;
		}
	    }
	    else if(event.type == SDL_EVENT_KEY_DOWN)
	    {
		if(event.key.key == SDLK_ESCAPE)
		{
		    break;
		}
		if(event.key.key == SDLK_W)
		{
		    gameState.gameInput.up = 1;
		}
		else if(event.key.key == SDLK_S)
		{
		    gameState.gameInput.down = 1;

		}
		else if(event.key.key == SDLK_A)
		{
		    gameState.gameInput.left = 1;

		}
		else if(event.key.key == SDLK_D)
		{
		    gameState.gameInput.right = 1;

		}
	    }
	    else
	    {
		gameState.gameInput.up = 0;
		gameState.gameInput.down = 0;
		gameState.gameInput.right = 0;
		gameState.gameInput.left = 0;

	    }
	    continue;
	}
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	uint64_t currTick = SDL_GetTicks();

	float delta = (currTick - lastTick) / 1000.0f;

	gameState.delta = delta;
	gameState.ticks = currTick;
	GameUpdateAndRender();

	void *start = gameState.commandBuffer.base;
	while (start < gameState.commandBuffer.end)
	{
	    RENDER_CMD *type = (RENDER_CMD *)start;
	    switch(*type)
	    {
		case RENDER_RECT:
		    render_cmd_rect *rect = (render_cmd_rect*)start;
		    SDL_SetRenderDrawColor(renderer, rect->r, rect->g, rect->b, rect->a);
		    SDL_FRect sdl_rect;
		    sdl_rect.x = rect->x;
		    sdl_rect.y = rect->y;
		    sdl_rect.w = rect->w;
		    sdl_rect.h = rect->h;
		    SDL_RenderFillRect(renderer, &sdl_rect);
		    start = rect + 1;
		    break;
		case RENDER_LINE:
		    render_cmd_line *line = (render_cmd_line*)start;
		    SDL_SetRenderDrawColor(renderer, line->r, line->g, line->b, line->a);
		    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		    SDL_RenderLine(renderer, line->x1, line->y1, line->x2, line->y2);
		    start = line + 1;
		    break;
		case RENDER_CLEAR:
		    render_cmd_clear *clear = (render_cmd_clear*)start;
		    SDL_SetRenderDrawColor(renderer, clear->r, clear->g, clear->b, clear->a);
		    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		    SDL_RenderClear(renderer);
		    start = clear + 1;
		    break;
		case RENDER_TEXTURE:
		    render_cmd_texture *texture = (render_cmd_texture*)start;
		    SDL_FRect source = textureCoords[texture->id];
		    SDL_FRect dest = {texture->x, texture->y, texture->w, texture->h};
		    SDL_RenderTexture(renderer, atlas, &source, &dest);
		    start = texture + 1;
		    break;
		case RENDER_TEXT:
		    {
			render_cmd_text *text = (render_cmd_text*)start;
			SDL_Color foreground = {text->r, text->g, text->b};
			SDL_Surface *textSurface = TTF_RenderText_Solid(font, text->message, 0, foreground);
			SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
			SDL_SetTextureScaleMode(textTexture, SDL_SCALEMODE_NEAREST);
			SDL_FRect dest = {text->x, text->y, textSurface->w * text->fontScale, textSurface->h * text->fontScale};
			SDL_RenderTexture(renderer, textTexture, NULL, &dest);
			SDL_DestroySurface(textSurface);
			SDL_DestroyTexture(textTexture);
			start = text + 1;
			break;
		    }
		default:
		    Assert(0 && "This shouldn't happen");
		    break;

	    }

	}

	gameState.commandBuffer.end = gameState.commandBuffer.base;
	gameState.gameInput.mouseUp = 0;

	SDL_RenderPresent(renderer);

	lastTick = currTick;

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
