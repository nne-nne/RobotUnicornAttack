#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#include<math.h>
#include<stdio.h>
#include<string.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

/*all distances/dimensions in pixels
double converted to int in runtime*/
#define SCREEN_WIDTH			640  
#define SCREEN_HEIGHT			480
#define POS_STEP				500.0f	//delta position per frame in naive control mode

#define UNICORN_HEIGHT			60.0f	//predefined sizes of objects
#define UNICORN_WIDTH			90.0f
#define LONG_TILE_WIDTH			800.0f
#define SHORT_TILE_WIDTH		360.0f
#define TILE_HEIGHT				4.0f
#define BASE_GAMEOBJECT_SIZE	90.0f
#define SMALL_GAMEOBJECT_SIZE	60.0f
#define BACKGROUND_WIDTH		800.0f
#define BACKGROUND_HEIGHT		600.0f
#define VERTICAL_OBSTACLE_LENGTH 190.0f

#define UNICORN_X				90.0f		//unicorn's distance from left edge of the screen
#define MAX_LIVES				3
#define SKY_SCROLL_SPEED		5
#define CLOUDS_SCROLL_SPEED		50
#define Y_SCROLL_MARGIN			160			//distance from any horizontal edge of the screen where y-scrolling starts
#define FOREGROUND_SCROLL_SPEED	200
#define BACKGROUND_LOOP_ELEMENTS 2			//the number of elements sufficient to fill background
#define UNICORN_STARTING_SPEED	1.0f
#define UNICORN_ACCELERATION	0.1f
#define SCORE_MULTIPLIER		3.0f
#define GRAVITY					1200.0f
#define MAX_JUMP_BONUS			2.0f
#define JUMPS_NUMBER			2
#define DASHES_NUMBER			1
#define JUMPS_AFTER_DASH		1
#define JUMP_STEP				5000.0f
#define JUMP_BASE_FORCE			300.0f
#define JUMP_EXTRA_FORCE		14.0f
#define DASH_SPEED				7.0f
#define DASH_TIME				0.8f
#define GROUND_MARGIN_MIN		1.0f
#define GROUND_MARGIN_MAX		2.0f
#define FRONT_MARGIN			2.0f
#define BOTTOM_COLLIDER_Y		1000
#define TIME_BEFORE_RESET		4.0f
#define LIVES_X_STEP			50
#define LIVES_Y_MARGIN			30
#define MILESTONE_DISTANCE		100.0f
#define DOLPHIN_SCREEN_TIME		5.0f
#define STAR_BONUS				10
#define FAIRY_BONUS				15
#define BONUS_ANIMATION_X_SPEED	SCREEN_WIDTH/5
#define BONUS_ANIMATION_Y_SPEED 3
#define BONUS_ANIMATION_Y_AMP	0.5f



#pragma region Enums
enum Surfaces
{
	CHARSET,
	ETI,
	DZIUNIA1,
	DZIUNIA2,
	DZIUNIA2_1,
	DZIUNIA3,
	DZIUNIA3_1,
	DZIUNIA4,
	DZIUNIADEAD1,
	DZIUNIADEAD2,
	DZIUNIADEAD3,
	DZIUNIADEAD4,
	BASE_TILE,
	LITTLE_TILE,
	PURPLE_TILE,
	THRESHOLD,
	STALAKTYT,
	SKY,
	CLOUDS,
	DELFIN1,
	DELFIN2,
	STAR1,
	STAR2,
	STAREXP1,
	STAREXP2,
	STAREXP3,
	STAREXP4,
	FAIRY,
	HEART,
	SURFACES_COUNT
};

enum PlayerState
{
	RUN,
	JUMP_UP,
	FALL,
	DASH,
	DEAD,
	UNICORN_STATES_COUNT
};

enum LevelLayer
{
	L_BASE_TILES,
	L_LITTLE_TILES,
	L_PURPLE_TILES,
	L_THRESHOLDS,
	L_ETI_OBSTACLES,
	L_HIGH_OBSTACLES,
	L_STARS,
	L_FAIRIES,
	LEVEL_LAYERS_COUNT
};

enum AppState
{
	MENU,
	CONTINUE,
	RESET,
	PAUSED,
	QUIT
};

enum Controls
{
	DEFAULT,
	NAIVE
};

enum Colors
{
	BLACK,
	GREEN,
	RED,
	BLUE,
	PINK,
	PINKYWHITE,
	COLORS_COUNT
};

enum Animations
{
	DZIUNIA_RUN,
	DZIUNIA_JUMP,
	DZIUNIA_FALL,
	DZIUNIA_DASH,
	DZIUNIA_DEAD,
	DOLPHIN,
	STAR,
	STAR_EXPLODE,
	ANIMATIONS_COUNT
};

enum MenuOptions
{
	LV1,
	LV2,
	LV3,
	EXIT,
	MENU_OPTIONS_COUNT,
};
#pragma endregion

#pragma region Structs

struct ScrRend
{
	SDL_Surface *screen;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	int rc;
};

struct Time
{
	int t1;
	int t2;
	int frames;
	double delta;
	double worldTime;
	double fps;
	double fpsTimer;
	double timeScale;
};

struct Coordinates
{
	double x, y;
};

struct Animation
{
	SDL_Surface **frames;
	int fps;
	int framesCount;
};

struct Clip
{
	Animation *animation;
	double startingTime;
	bool played;
};

struct GameObject
{
	Coordinates position;
	Coordinates dimensions;
	SDL_Surface *surface;
	Clip clip;
};

struct Level
{
	GameObject sky[BACKGROUND_LOOP_ELEMENTS];
	GameObject clouds[BACKGROUND_LOOP_ELEMENTS];
	double length;
	GameObject *elemArrays[LEVEL_LAYERS_COUNT];
	GameObject bottomCollider;
	int elemsCount[LEVEL_LAYERS_COUNT];
};

struct Player
{
	Coordinates deltaPos;
	double speedX, speedY;
	int jumpsLeft;
	int dashesLeft;
	double jumpTimer;
	double dashTimer;
	int lives;
	GameObject unicorn;
	PlayerState playerState;
	PlayerState targetPlayerState;
};

struct Bonus
{
	Coordinates position;
	int value;
};

struct GameState
{
	double score;
	Time time;
	Level level;
	Bonus bonus;
	double nextMileStone;
	double dolphinsScreenTime;
};

struct App
{
	Animation *animations;
	Player player;
	AppState appState;
	Controls controlsMode;
	MenuOptions activeOption;
	SDL_Event event;
	ScrRend scrrend;
	SDL_Surface **surfaces;
	int *colors;
};


#pragma endregion

#pragma region SDL_Drawing
// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
// charset to bitmapa 128x128 zawierajπca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset)
{
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text)
	{
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	}
}


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y)
{
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
}


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color)
{
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
}


// rysowanie linii o d≥ugoúci l w pionie (gdy dx = 0, dy = 1) 
// bπdü poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color)
{
	for (int i = 0; i < l; i++)
	{
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	}
}


// rysowanie prostokπta o d≥ugoúci bokÛw l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor)
{
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
	{
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	}
}
#pragma endregion

#pragma region SDL_ScreenOperations

bool SDL_Initialization(ScrRend *scrrend, int* rc)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL_Init error: %s\n", SDL_GetError());
		return false;
	}

	// tryb pe≥noekranowy / fullscreen mode
	//*rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &scrrend->window, &scrrend->renderer);
	*rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &scrrend->window, &scrrend->renderer);
	if (*rc != 0)
	{
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return false;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(scrrend->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(scrrend->renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(scrrend->window, "Adam Leczkowski 180280");
	scrrend->screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	scrrend->scrtex = SDL_CreateTexture(scrrend->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	// wy≥πczenie widocznoúci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);
	return true;
}

void RenderInfoText(SDL_Surface *screen, int* colors, GameState *gameState, SDL_Surface **surfaces)
{
	char text[128];
	DrawRectangle(screen, 40, 10, SCREEN_WIDTH - 80, 36, colors[PINKYWHITE], colors[PINK]);
	sprintf(text, "Adam Leczkowski 180280, score:%.01f", gameState->score);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 16, text, surfaces[CHARSET]);
	sprintf(text, "worldTime: = %.1lf s  %.0lf fps", gameState->time.worldTime, gameState->time.fps);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 32, text, surfaces[CHARSET]);
}

void RenderGameOverText(SDL_Surface *screen, int* colors, GameState *gameState, SDL_Surface **surfaces)
{
	char text[128];
	DrawRectangle(screen, SCREEN_WIDTH / 2 - 220, SCREEN_HEIGHT / 2 - 40, 440, 80, colors[PINKYWHITE], colors[PINK]);
	sprintf(text, "It's over, Dziunia, your score is: %.01f", gameState->score);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2 - 10, text, surfaces[CHARSET]);
	sprintf(text, "Do you want to continue chasing your dreams? Y/N");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2 + 10, text, surfaces[CHARSET]);
}

void RenderPauseText(SDL_Surface *screen, int* colors, GameState *gameState, SDL_Surface **surfaces)
{
	char text[128];
	DrawRectangle(screen, SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT/2 - 20, 400, 40, colors[PINKYWHITE], colors[PINK]);
	sprintf(text, "Game paused, press 'p' to continue");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 2, text, surfaces[CHARSET]);
}

void RenderMenuOptions(App *app, SDL_Surface *screen, int* colors, SDL_Surface **surfaces)
{
	char *optionTexts[] = {
		"Level 1",
		"Level 2",
		"Level 3",
		"Exit",
	};
	char text[128];
	for (int option_i = 0; option_i < MENU_OPTIONS_COUNT; option_i++)
	{
		Colors fill = option_i == app->activeOption ? PINK : BLACK;
		DrawRectangle(screen, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 4 + 30 + option_i * 30, 100, 20, colors[PINKYWHITE], colors[fill]);
		sprintf(text, optionTexts[option_i]);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 4 + 35 + option_i * 30, text, surfaces[CHARSET]);
	}
}

void RenderMainMenu(App *app, GameState *gameState, SDL_Surface *screen, SDL_Surface **surfaces, int *colors)
{
	char text[128];
	DrawSurface(screen, surfaces[SKY], SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	DrawRectangle(screen, SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 4 - 20, 100, 40, colors[PINKYWHITE], colors[PINK]);
	DrawSurface(screen, surfaces[STALAKTYT], SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 4 + 20);
	DrawSurface(screen, surfaces[STALAKTYT], SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT / 4 + 20);
	DrawSurface(screen, surfaces[DZIUNIA1], SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4 - 50);
	sprintf(text, "MAIN MENU");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, screen->h / 4, text, surfaces[CHARSET]);
	RenderMenuOptions(app, screen, colors, surfaces);
}

void RenderBonus(SDL_Surface *screen, Bonus *bonus, SDL_Surface **surfaces)
{
	char text[128];
	sprintf(text, "+%d", bonus->value);
	DrawString(screen, (int)bonus->position.x, (int)bonus->position.y, text, surfaces[CHARSET]);
}

void UpdateScreen(ScrRend *scrrend)
{
	SDL_UpdateTexture(scrrend->scrtex, NULL, scrrend->screen->pixels, scrrend->screen->pitch);
	SDL_RenderCopy(scrrend->renderer, scrrend->scrtex, NULL, NULL);
	SDL_RenderPresent(scrrend->renderer);
}
#pragma endregion

#pragma region Resources

bool LoadAllBMPs(SDL_Surface **surfaces, int n, ScrRend *scrRend)
{
	char *paths[] = {
		"./cs8x8.bmp",
		"./eti.bmp",
		"./dziunia1.bmp",
		"./dziunia2.bmp",
		"./dziunia2-1.bmp",
		"./dziunia3.bmp",
		"./dziunia3-1.bmp",
		"./dziunia4.bmp",
		"./dziuniadead1.bmp",
		"./dziuniadead2.bmp",
		"./dziuniadead3.bmp",
		"./dziuniadead4.bmp",
		"./basetile.bmp",
		"./littleTile.bmp",
		"./purpleTile.bmp",
		"./threshold.bmp",
		"./stalaktyt.bmp",
		"./sky.bmp",
		"./clouds.bmp",
		"./star1.bmp",
		"./star2.bmp",
		"./star1.bmp",
		"./star2.bmp",
		"./starexp1.bmp",
		"./starexp2.bmp",
		"./starexp3.bmp",
		"./starexp4.bmp",
		"./wrozka.bmp",
		"./heart.bmp"
	};
	for (int surf_i = 0; surf_i < n; surf_i++)
	{
		surfaces[surf_i] = SDL_LoadBMP(paths[surf_i]);
		if (surfaces[surf_i] == NULL)
		{
			printf("SDL_LoadBMP(%s) error: %s\n", paths[surf_i], SDL_GetError());
			SDL_FreeSurface(scrRend->screen);
			SDL_DestroyTexture(scrRend->scrtex);
			SDL_DestroyWindow(scrRend->window);
			SDL_DestroyRenderer(scrRend->renderer);
			SDL_Quit();
			return false;
		}
		printf("loaded %s\n", paths[surf_i]);
	}
	return true;
}

void FreeLevelElements(Level *level)
{
	for (int group_i = 0; group_i < LEVEL_LAYERS_COUNT; group_i++)
	{
		if (level->elemsCount[group_i] > 0) free(level->elemArrays[group_i]);
	}
}

void FreeResources(App *app, GameState *gameState)
{
	for (int anim_i = 0; anim_i < ANIMATIONS_COUNT; anim_i++)
	{
		free(app->animations[anim_i].frames);
	}
	free(app->animations);
}

void CloseSDL(ScrRend* scrrend)
{
	SDL_FreeSurface(scrrend->screen);
	SDL_DestroyTexture(scrrend->scrtex);
	SDL_DestroyRenderer(scrrend->renderer);
	SDL_DestroyWindow(scrrend->window);
	SDL_Quit();
}

void Close(App *app, GameState *gameState)
{
	FreeLevelElements(&gameState->level);
	FreeResources(app, gameState);
	free(app->colors);
	for (int surf_i = 0; surf_i < SURFACES_COUNT; surf_i++)
	{
		SDL_FreeSurface(app->surfaces[surf_i]);
	}
	CloseSDL(&app->scrrend);
}

void AssignColors(int *colors, SDL_Surface *screen)
{
	colors[BLACK] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	colors[GREEN] = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	colors[RED] = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	colors[BLUE] = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	colors[PINK] = SDL_MapRGB(screen->format, 0xFC, 0x90, 0xEA);
	colors[PINKYWHITE] = SDL_MapRGB(screen->format, 0xFC, 0xD4, 0xF6);
}
#pragma endregion

#pragma region Time

void StartClock(Time *time)
{
	time->t1 = SDL_GetTicks();
}

void ResetTimer(Time *time)
{
	time->frames = 0;
	time->fpsTimer = 0;
	time->worldTime = 0;
	time->timeScale = 1;
}

void ManageTime(Time *time)
{
	time->t2 = SDL_GetTicks();
	time->delta = (time->t2 - time->t1) * 0.001 * time->timeScale;
	time->t1 = time->t2;
	time->worldTime += time->delta;
}

void ManageFPS(Time *time)
{
	time->fpsTimer += time->delta;
	if (time->fpsTimer > 0.5)
	{
		time->fps = time->frames * 2;
		time->frames = 0;
		time->fpsTimer -= 0.5;
	}
}

#pragma endregion

#pragma region RenderingGameObjects

int GetMilestones(GameState *gameState)
{
	return (int)(gameState->nextMileStone / MILESTONE_DISTANCE - 1); //next -1 is current
}

void Animate(GameObject *gameObject, Animation *animation, double time)
{
	int frame = (int)(time * animation->fps) % animation->framesCount;
	gameObject->surface = animation->frames[frame];
}

void StartClip(Clip *clip, double startingTime)
{
	clip->startingTime = startingTime;
	clip->played = true;
}

void PlayClip(GameObject *gameObject, double time)
{
	int frame = (int)((time - gameObject->clip.startingTime) * gameObject->clip.animation->fps);
	if (frame >= gameObject->clip.animation->framesCount) frame = gameObject->clip.animation->framesCount - 1; //last frame
	gameObject->surface = gameObject->clip.animation->frames[frame];
}

void AnimateUnicorn(GameObject *unicorn, PlayerState playerState, Animation *animations, double worldTime)
{
	if (unicorn->clip.played)
	{
		PlayClip(unicorn, worldTime);
		return;
	}
	switch (playerState)
	{
	case RUN:
		Animate(unicorn, &animations[DZIUNIA_RUN], worldTime);
		break;
	case JUMP_UP:
		Animate(unicorn, &animations[DZIUNIA_JUMP], worldTime);
		break;
	case FALL:
		Animate(unicorn, &animations[DZIUNIA_FALL], worldTime);
		break;
	case DASH:
		Animate(unicorn, &animations[DZIUNIA_DASH], worldTime);
		break;
	case DEAD:
		Animate(unicorn, &animations[DZIUNIA_DEAD], worldTime);
		break;
	default:
		printf("animation error\n");
		break;
	}
}

void AnimateGameObjects(Level *lv, Animation *animations, double worldTime)
{
	for (int star_i = 0; star_i < lv->elemsCount[L_STARS]; star_i++)
	{
		if (lv->elemArrays[L_STARS][star_i].clip.played)
		{
			PlayClip(&lv->elemArrays[L_STARS][star_i], worldTime);
		}
		else
		{
			Animate(&lv->elemArrays[L_STARS][star_i], &animations[STAR], worldTime);
		}
	}
}

void DrawGameObject(SDL_Surface *screen, GameObject *gameObject)
{
	DrawSurface(screen, gameObject->surface, (int)gameObject->position.x, (int)gameObject->position.y);
}

void DrawLayer(SDL_Surface *screen, GameObject *layer, int n)
{
	for (int obj_i = 0; obj_i < n; obj_i++)
	{
		DrawGameObject(screen, &layer[obj_i]);
	}
}

void DrawLevel(SDL_Surface *screen, Level *lv)
{
	DrawLayer(screen, lv->sky, BACKGROUND_LOOP_ELEMENTS);
	DrawLayer(screen, lv->clouds, BACKGROUND_LOOP_ELEMENTS);
	for (int layer_i = 0; layer_i < LEVEL_LAYERS_COUNT; layer_i++)
	{
		DrawLayer(screen, lv->elemArrays[layer_i], lv->elemsCount[layer_i]);
	}
}

void DrawLives(App *app, SDL_Surface *screen, SDL_Surface *liveSprite)
{
	for (int live_i = 0; live_i < app->player.lives; live_i++)
	{
		DrawSurface(screen, liveSprite, (int)(UNICORN_X) + live_i * LIVES_X_STEP, SCREEN_HEIGHT - LIVES_Y_MARGIN);
	}
}

void DrawDolphins(SDL_Surface *screen, GameState *gameState, Animation *animations)
{
	if (gameState->dolphinsScreenTime > 0.0f)
	{
		for (int dolphin_i = 0; dolphin_i < GetMilestones(gameState); dolphin_i++)
		{
			double dolphinPivotX = SCREEN_WIDTH - (SCREEN_WIDTH + 2 * BASE_GAMEOBJECT_SIZE) / DOLPHIN_SCREEN_TIME * gameState->dolphinsScreenTime;
			int frame = ((int)(gameState->dolphinsScreenTime * 100) + dolphin_i) * animations[DOLPHIN].fps % 2;
			DrawSurface(screen, animations[DOLPHIN].frames[frame], (int)(dolphinPivotX + dolphin_i * BASE_GAMEOBJECT_SIZE), SCREEN_HEIGHT + (int)(sin(dolphinPivotX / 30 + dolphin_i * M_PI / 6) * SCREEN_HEIGHT/6));
		}
		gameState->dolphinsScreenTime -= gameState->time.delta;
	}
}

void AnimateBonus(GameState *gameState)
{
	gameState->bonus.position.x -= gameState->time.delta * BONUS_ANIMATION_X_SPEED;
	gameState->bonus.position.y += BONUS_ANIMATION_Y_AMP * sin(BONUS_ANIMATION_Y_SPEED * gameState->time.worldTime);
}

void RenderBMPs(App *app, GameState *gameState, SDL_Surface *screen, SDL_Surface **surfaces)
{
	AnimateUnicorn(&app->player.unicorn, app->player.playerState, app->animations, gameState->time.worldTime);
	AnimateGameObjects(&gameState->level, app->animations, gameState->time.worldTime);
	DrawLevel(screen, &gameState->level);
	DrawGameObject(screen, &app->player.unicorn);
	DrawLives(app, screen, surfaces[HEART]);
	DrawDolphins(screen, gameState, app->animations);
	AnimateBonus(gameState);
	RenderBonus(screen, &gameState->bonus, surfaces);
}

#pragma endregion

#pragma region GameInitialization

//if source is not null, assign frames from automatically
void InitializeAnimation(Animation *anim, int fps, int framesCount, SDL_Surface **source)
{
	anim->framesCount = framesCount;
	anim->fps = fps;
	anim->frames = (SDL_Surface **)malloc(framesCount * sizeof(SDL_Surface *));
	if (source == NULL) return;

	for (int frame_i = 0; frame_i < framesCount; frame_i++)
	{
		anim->frames[frame_i] = source[frame_i];
	}
}

void InitializeClips(Level *lv, Animation *animations, GameObject *unicorn)
{
	unicorn->clip.animation = &animations[DZIUNIA_DEAD];
	for (int star_i = 0; star_i < lv->elemsCount[L_STARS]; star_i++)
	{
		lv->elemArrays[L_STARS]->clip.animation = &animations[STAR_EXPLODE];
	}
}

void InitializeResources(App *app, SDL_Surface **surfaces)
{
	app->animations = (Animation *)malloc(ANIMATIONS_COUNT * sizeof(Animation));
	InitializeAnimation(&app->animations[DZIUNIA_RUN], 12, 3, NULL);
	app->animations[DZIUNIA_RUN].frames[0] = surfaces[DZIUNIA1];
	app->animations[DZIUNIA_RUN].frames[1] = surfaces[DZIUNIA2];
	app->animations[DZIUNIA_RUN].frames[2] = surfaces[DZIUNIA3];
	InitializeAnimation(&app->animations[DZIUNIA_JUMP], 8, 2, &surfaces[DZIUNIA2]);
	InitializeAnimation(&app->animations[DZIUNIA_FALL], 8, 2, &surfaces[DZIUNIA3]);
	InitializeAnimation(&app->animations[DZIUNIA_DASH], 1, 1, &surfaces[DZIUNIA4]);
	InitializeAnimation(&app->animations[DZIUNIA_DEAD], 8, 4, &surfaces[DZIUNIADEAD1]);
	InitializeAnimation(&app->animations[DOLPHIN], 3, 2, &surfaces[DELFIN1]);
	InitializeAnimation(&app->animations[STAR], 3, 2, &surfaces[STAR1]);
	InitializeAnimation(&app->animations[STAR_EXPLODE], 8, 4, &surfaces[STAREXP1]);
}

void ResetPlayer(Player *player)
{
	player->unicorn.position.x = UNICORN_X;
	player->unicorn.position.y = SCREEN_HEIGHT / 2 - UNICORN_HEIGHT;
	player->speedX = UNICORN_STARTING_SPEED;
	player->speedY = 0.0f;
	player->jumpTimer = MAX_JUMP_BONUS;
	player->dashTimer = DASH_TIME;
	player->jumpsLeft = JUMPS_NUMBER;
	player->dashesLeft = DASHES_NUMBER;
	player->playerState = RUN;
	player->targetPlayerState = RUN;
	player->deltaPos = { 0.0f, 0.0f };
}

void ResetGameState(GameState *gameState)
{
	ResetTimer(&gameState->time);
	gameState->score = 0.0f;
	gameState->bonus.position = { -1.0f, -1.0f };
	gameState->bonus.value = 0;
	gameState->nextMileStone = MILESTONE_DISTANCE;
}

void InitializeGameObject(GameObject *gameObject, SDL_Surface *surface, Coordinates position, Coordinates dimensions)
{
	gameObject->position = position;
	gameObject->dimensions = dimensions;
	gameObject->surface = surface;
	gameObject->clip.animation = NULL;
	gameObject->clip.startingTime = 0.0f;
	gameObject->clip.played = false;
}

void InitializeSkyAndClouds(Level *level, SDL_Surface** surfaces)
{
	for (int sky_i = 0; sky_i < BACKGROUND_LOOP_ELEMENTS; sky_i++)
	{
		Coordinates position = { SCREEN_WIDTH / 2 + sky_i * BACKGROUND_WIDTH , SCREEN_HEIGHT / 2 };
		Coordinates dimensions = { BACKGROUND_WIDTH, BACKGROUND_HEIGHT };
		InitializeGameObject(&level->sky[sky_i], surfaces[SKY], position, dimensions);
	}
	for (int cloud_i = 0; cloud_i < BACKGROUND_LOOP_ELEMENTS; cloud_i++)
	{
		Coordinates position = { SCREEN_WIDTH / 2 + cloud_i * BACKGROUND_WIDTH, SCREEN_HEIGHT / 2 };
		Coordinates dimensions = { BACKGROUND_WIDTH, BACKGROUND_HEIGHT };
		InitializeGameObject(&level->clouds[cloud_i], surfaces[CLOUDS], position, dimensions);
	}
}

void LoadLevel1(SDL_Surface** surfaces, Level *lv)
{
	int params[LEVEL_LAYERS_COUNT] = { 2,0,0,0,1,0,0,0 };
	for (int group_i = 0; group_i < LEVEL_LAYERS_COUNT; group_i++)
	{
		lv->elemsCount[group_i] = params[group_i];
		lv->elemArrays[group_i] = (GameObject *)malloc(lv->elemsCount[group_i] * sizeof(GameObject));
	}
	lv->length = 2 * LONG_TILE_WIDTH;
	InitializeGameObject(&lv->elemArrays[L_BASE_TILES][0], surfaces[BASE_TILE], { 320, 240 + UNICORN_HEIGHT / 2 }, { LONG_TILE_WIDTH, TILE_HEIGHT });
	InitializeGameObject(&lv->elemArrays[L_BASE_TILES][1], surfaces[BASE_TILE], { 1120, 240 + UNICORN_HEIGHT / 2 }, { LONG_TILE_WIDTH, TILE_HEIGHT });
	InitializeGameObject(&lv->elemArrays[L_ETI_OBSTACLES][0], surfaces[ETI], { 960, 230 }, { BASE_GAMEOBJECT_SIZE, BASE_GAMEOBJECT_SIZE });
	InitializeGameObject(&lv->bottomCollider, NULL, { 0, 1000 }, { lv->length, BASE_GAMEOBJECT_SIZE });

}

void LoadLevel2(SDL_Surface** surfaces, Level *lv)
{
	int params[LEVEL_LAYERS_COUNT] = { 1,1,3,1,1,1,1,1 };
	for (int group_i = 0; group_i < LEVEL_LAYERS_COUNT; group_i++)
	{
		lv->elemsCount[group_i] = params[group_i];
		lv->elemArrays[group_i] = (GameObject *)malloc(lv->elemsCount[group_i] * sizeof(GameObject));
	}
	lv->length = 3 * LONG_TILE_WIDTH + 2 * SHORT_TILE_WIDTH + 90;
	InitializeGameObject(&lv->elemArrays[L_BASE_TILES][0], surfaces[BASE_TILE], { SCREEN_WIDTH / 2, 240 + UNICORN_HEIGHT / 2 }, { LONG_TILE_WIDTH, TILE_HEIGHT });
	InitializeGameObject(&lv->elemArrays[L_LITTLE_TILES][0], surfaces[LITTLE_TILE], { SCREEN_WIDTH / 2 + LONG_TILE_WIDTH, 240 + UNICORN_HEIGHT / 2 }, { SHORT_TILE_WIDTH, TILE_HEIGHT });
	InitializeGameObject(&lv->elemArrays[L_PURPLE_TILES][0], surfaces[PURPLE_TILE], { SCREEN_WIDTH / 2 + LONG_TILE_WIDTH + SHORT_TILE_WIDTH, 240 + UNICORN_HEIGHT / 2 }, { LONG_TILE_WIDTH, TILE_HEIGHT });
	InitializeGameObject(&lv->elemArrays[L_PURPLE_TILES][1], surfaces[PURPLE_TILE], { SCREEN_WIDTH / 2 + 2 * LONG_TILE_WIDTH + 2 * SHORT_TILE_WIDTH, 240 + UNICORN_HEIGHT / 2 }, { LONG_TILE_WIDTH, TILE_HEIGHT });
	InitializeGameObject(&lv->elemArrays[L_PURPLE_TILES][2], surfaces[PURPLE_TILE], { SCREEN_WIDTH / 2 + 2 * LONG_TILE_WIDTH + 2 * SHORT_TILE_WIDTH, 0 }, { LONG_TILE_WIDTH, TILE_HEIGHT });
	InitializeGameObject(&lv->elemArrays[L_THRESHOLDS][0], surfaces[THRESHOLD], { SCREEN_WIDTH / 2 + 400, 210 }, { SHORT_TILE_WIDTH, TILE_HEIGHT });
	InitializeGameObject(&lv->elemArrays[L_ETI_OBSTACLES][0], surfaces[ETI], { SCREEN_WIDTH * 3 / 2, 230 }, { BASE_GAMEOBJECT_SIZE, BASE_GAMEOBJECT_SIZE });
	InitializeGameObject(&lv->elemArrays[L_HIGH_OBSTACLES][0], surfaces[STALAKTYT], { SCREEN_WIDTH / 2 + LONG_TILE_WIDTH * 3 / 2, 10 }, { BASE_GAMEOBJECT_SIZE, VERTICAL_OBSTACLE_LENGTH });
	InitializeGameObject(&lv->elemArrays[L_STARS][0], surfaces[STAR2], { SCREEN_WIDTH / 2 + 2 * LONG_TILE_WIDTH, 230 }, { SMALL_GAMEOBJECT_SIZE, SMALL_GAMEOBJECT_SIZE });
	InitializeGameObject(&lv->elemArrays[L_FAIRIES][0], surfaces[FAIRY], { 1050, 230 }, { BASE_GAMEOBJECT_SIZE, BASE_GAMEOBJECT_SIZE });
	InitializeGameObject(&lv->bottomCollider, NULL, { 0, 1000 }, { lv->length, GROUND_MARGIN_MAX });
}

void LoadLevel3(SDL_Surface** surfaces, Level *lv)
{
	int params[LEVEL_LAYERS_COUNT] = { 8,4,0,0,3,2,0,0 };
	for (int group_i = 0; group_i < LEVEL_LAYERS_COUNT; group_i++)
	{
		lv->elemsCount[group_i] = params[group_i];
		lv->elemArrays[group_i] = (GameObject *)malloc(lv->elemsCount[group_i] * sizeof(GameObject));
	}
	lv->length = 12 * LONG_TILE_WIDTH;
	//spawn tile

	InitializeGameObject(&lv->elemArrays[L_BASE_TILES][0], surfaces[BASE_TILE], { LONG_TILE_WIDTH * 0.5f, LONG_TILE_WIDTH * 0.3f}, { LONG_TILE_WIDTH, TILE_HEIGHT });

	for (int tile_i = 1; tile_i < params[L_BASE_TILES]; tile_i++)
	{
		InitializeGameObject(&lv->elemArrays[L_BASE_TILES][tile_i], surfaces[BASE_TILE], {  1.5f * tile_i * LONG_TILE_WIDTH , LONG_TILE_WIDTH * 0.3f + 0.2f * BASE_GAMEOBJECT_SIZE * ((149 * tile_i) % 11)}, { LONG_TILE_WIDTH, TILE_HEIGHT });
	}
	for (int tile_i = 0; tile_i < params[L_LITTLE_TILES]; tile_i++)
	{
		InitializeGameObject(&lv->elemArrays[L_LITTLE_TILES][tile_i], surfaces[LITTLE_TILE], { SHORT_TILE_WIDTH * ((23 * (tile_i + 1)) % 19 ), LONG_TILE_WIDTH * 0.2f + BASE_GAMEOBJECT_SIZE * ((149 * tile_i) % 3) }, { SHORT_TILE_WIDTH, TILE_HEIGHT });
	}
	for (int tile_i = 0; tile_i < params[L_ETI_OBSTACLES]; tile_i++)
	{
		InitializeGameObject(&lv->elemArrays[L_ETI_OBSTACLES][tile_i], surfaces[ETI], { lv->elemArrays[L_BASE_TILES][3 * tile_i + 1].position.x + LONG_TILE_WIDTH * 2 / 5, lv->elemArrays[L_BASE_TILES][3 * tile_i + 1].position.y - BASE_GAMEOBJECT_SIZE / 2}, { BASE_GAMEOBJECT_SIZE, BASE_GAMEOBJECT_SIZE });
	}
	for (int tile_i = 0; tile_i < params[L_HIGH_OBSTACLES]; tile_i++)
	{
		InitializeGameObject(&lv->elemArrays[L_HIGH_OBSTACLES][tile_i], surfaces[STALAKTYT], { (tile_i + 1) * 3 * LONG_TILE_WIDTH, 10 }, { BASE_GAMEOBJECT_SIZE, VERTICAL_OBSTACLE_LENGTH });
	}
	InitializeGameObject(&lv->bottomCollider, NULL, { 0, 4 * LONG_TILE_WIDTH }, { lv->length, GROUND_MARGIN_MAX });
}

void InitializeLevel(App *app, GameState *gameState, SDL_Surface** surfaces, Level* lv, int levelId)
{
	ResetGameState(gameState);
	ResetPlayer(&app->player);
	InitializeSkyAndClouds(&gameState->level, surfaces);
	InitializeGameObject(&app->player.unicorn, surfaces[DZIUNIA1], { UNICORN_X , SCREEN_HEIGHT / 2 - UNICORN_HEIGHT }, { UNICORN_WIDTH,UNICORN_HEIGHT });
	switch (levelId)
	{
	case 0:
		LoadLevel1(surfaces, lv);
		break;
	case 1:
		LoadLevel2(surfaces, lv);
		break;
	case 2: 
		LoadLevel3(surfaces, lv);
		break;
	default:
		printf("level loading error\n");
		break;
	}
	InitializeClips(lv, app->animations, &app->player.unicorn);
}

#pragma endregion

#pragma region Collisions

bool Overlaps(GameObject *s1, GameObject *s2)
{
	if (fabs(s1->position.x - s2->position.x) < (s1->dimensions.x + s2->dimensions.x) / 2
		&& fabs(s1->position.y - s2->position.y) < (s1->dimensions.y + s2->dimensions.y) / 2)
			return true;
	else return false;
}

bool AnyOfLayerOverlaps(GameObject *unicorn, Level *level, Coordinates margin, LevelLayer layer)
{
	unicorn->position.y += margin.y;
	unicorn->position.x += margin.x;
	for (int col_i = 0; col_i < level->elemsCount[layer]; col_i++)
	{
		if (Overlaps(unicorn, &level->elemArrays[layer][col_i]))
		{
			unicorn->position.y -= margin.y;
			unicorn->position.x -= margin.x;

			return true;
		}
	}
	unicorn->position.y -= margin.y;
	unicorn->position.x -= margin.x;
	return false;
}

GameObject *OverlappingGroupMember(GameObject *unicorn, Level *level, Coordinates margin, LevelLayer layer)
{
	unicorn->position.y += margin.y;
	unicorn->position.x += margin.x;
	for (int col_i = 0; col_i < level->elemsCount[layer]; col_i++)
	{
		if (Overlaps(unicorn, &level->elemArrays[layer][col_i]))
		{
			unicorn->position.y -= margin.y;
			unicorn->position.x -= margin.x;

			return &level->elemArrays[layer][col_i];
		}
	}
}

bool DetectGround(App *app, GameState *gameState, Coordinates margin)
{
	int groundLayersMask[LEVEL_LAYERS_COUNT] = {1, 1, 1, 1, 0, 0, 0, 0};
	for (int layer_i = 0; layer_i < LEVEL_LAYERS_COUNT; layer_i++)
	{
		if (AnyOfLayerOverlaps(&app->player.unicorn, &gameState->level, margin, (LevelLayer)layer_i))
		{
			if(groundLayersMask[layer_i]) return true;
		}
	}
	return false;
}

bool DetectObstacleCollision(App *app, GameState *gameState)
{
	int obstacleLayersMask[LEVEL_LAYERS_COUNT] = { 0, 0, 0, 0, 1, 1, 0, 0 };
	for (int layer_i = 0; layer_i < LEVEL_LAYERS_COUNT; layer_i++)
	{
		if (AnyOfLayerOverlaps(&app->player.unicorn, &gameState->level, { 0.0f,0.0f }, (LevelLayer)layer_i))
		{
			if (obstacleLayersMask[layer_i]) return true;
		}
	}
	// dashing unicorn shoots stars
	if (app->player.playerState != DASH && AnyOfLayerOverlaps(&app->player.unicorn, &gameState->level, { 0.0f,0.0f }, L_STARS)) return true;

	if (Overlaps(&app->player.unicorn, &gameState->level.bottomCollider)) return true;

	return false;
}

bool DetectFrontTerrainCollision(App *app, GameState *gameState)
{
	int terrainLayersMask[LEVEL_LAYERS_COUNT] = { 1, 1, 1, 1, 0, 0, 0, 0 };
	for (int layer_i = 0; layer_i < LEVEL_LAYERS_COUNT; layer_i++)
	{
		if (AnyOfLayerOverlaps(&app->player.unicorn, &gameState->level, {FRONT_MARGIN, 0.0f }, (LevelLayer)layer_i))
		{
			if (terrainLayersMask[layer_i]) return true;
		}
	}
	return false;
}

void ActivateBonus(GameState *gameState, int value)
{
	gameState->bonus.value = value;
	gameState->bonus.position = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	gameState->score += value;
}

void DetectBonusCollision(App *app, GameState *gameState)
{
	if (app->player.playerState == DASH && AnyOfLayerOverlaps(&app->player.unicorn, &gameState->level, { 0.0f,0.0f }, L_STARS))
	{
		StartClip(&OverlappingGroupMember(&app->player.unicorn, &gameState->level, { 0.0f,0.0f }, L_STARS)->clip, gameState->time.delta);
		ActivateBonus(gameState, STAR_BONUS);
	}
	if (AnyOfLayerOverlaps(&app->player.unicorn, &gameState->level, { 0.0f,0.0f }, L_FAIRIES))
	{
		OverlappingGroupMember(&app->player.unicorn, &gameState->level, { 0.0f,0.0f }, L_FAIRIES)->position.y = BOTTOM_COLLIDER_Y;
		ActivateBonus(gameState, FAIRY_BONUS);
	}
}

#pragma endregion

#pragma region Controls

void Die(App *app, Player *player, double time)
{
	StartClip(&player->unicorn.clip, time);
	player->lives--;
	player->playerState = DEAD;
	player->speedX = 0.0f;
	if (player->lives <= 0) app->appState = MENU;
}

void Jump(Player *player)
{
	if (player->jumpsLeft > 0)
	{
		player->jumpsLeft--;
		player->speedY = -JUMP_BASE_FORCE;
	}
}

void ChangePlayerState(Player *player, PlayerState newState)
{
	if (player->playerState == DEAD) return; //dead unicorn is unable to perform actions

	if (newState == DASH)
	{
		if (player->dashesLeft > 0)
		{
			player->speedX += DASH_SPEED;
			player->jumpsLeft = JUMPS_AFTER_DASH;
			player->dashesLeft--;
		}
		else newState = FALL;
	}
	if (newState == JUMP_UP) Jump(player);

	if (player->playerState == DASH) player->speedX -= DASH_SPEED;

	player->playerState = newState;
}

void Accelerate(Player *player, double deltaTime)
{
	if (player->playerState != DASH && player->playerState != DEAD)
	{
		player->speedX += UNICORN_ACCELERATION * deltaTime;
	}
}

void MoveLevelY(GameObject* unicorn, Level *level, double val)
{
	unicorn->position.y += val;
	level->bottomCollider.position.y += val;
	for (int group_i = 0; group_i < LEVEL_LAYERS_COUNT; group_i++)
	{
		for (int elem_i = 0; elem_i <level->elemsCount[group_i]; elem_i++)
		{
			level->elemArrays[group_i][elem_i].position.y += val;
		}
	}
}

void GroundPlayer(Player *player)
{
	player->speedY = 0.0f;
	player->targetPlayerState = RUN;
	player->jumpsLeft = JUMPS_NUMBER;
	player->dashesLeft = DASHES_NUMBER;
}

void MoveUnicornY(App *app, GameState *gameState, GameObject *unicorn, double val, double deltaTime)
{
	//printf("moving unicorn y to %f\n", unicorn->position.y + val * deltaTime);
	double prevPos = unicorn->position.y;
	double targetPos = unicorn->position.y + val * deltaTime;
	if (targetPos > 0.0f && targetPos < SCREEN_HEIGHT) unicorn->position.y = targetPos;
	if (DetectGround(app, gameState, { 0.0f, GROUND_MARGIN_MIN }))
	{
		unicorn->position.y = prevPos;
		GroundPlayer(&app->player);
	}
	if (targetPos < Y_SCROLL_MARGIN || targetPos > SCREEN_HEIGHT - Y_SCROLL_MARGIN)
	{
		MoveLevelY(unicorn, &gameState->level, prevPos - targetPos);
	}
}

// in case horizontal movement of the unicorn on the screen is allowed
//void MoveUnicornX(GameObject *unicorn, double val, double deltaTime)
//{
//	double targetPos = unicorn->position.x + val * deltaTime;
//
//	if (targetPos > 0.0f && targetPos < SCREEN_WIDTH) unicorn->position.x = targetPos;
//}

void ManageDashing(Player *player, double deltaTime)
{
	player->dashTimer -= deltaTime;
	if(player->dashTimer < 0.0f)
	{ 
		player->dashTimer = DASH_TIME;
		player->targetPlayerState = FALL;
	}
}

void ManageJumping(Player *player, double deltaTime)
{
	if (player->jumpTimer > 0.0f)
	{
		player->speedY -= JUMP_EXTRA_FORCE;
		player->jumpTimer -= deltaTime * JUMP_STEP;
	}
	else
	{
		player->jumpTimer = MAX_JUMP_BONUS;
	}

	if (player->speedY >= 0.0f) //starting to fall
	{
		player->targetPlayerState = FALL;
	}
}

void NaiveControls(SDL_Event *event, Player *player)
{
	switch (event->type) {
	case SDL_KEYDOWN:
		if (event->key.keysym.sym == SDLK_UP) player->deltaPos.y = -POS_STEP;
		if (event->key.keysym.sym == SDLK_DOWN) player->deltaPos.y = POS_STEP;

		//in case horizontal movement of the Unicorn on the screen is allowed
		/*if (event->key.keysym.sym == SDLK_LEFT) unicorn->deltaPos.x = -POS_STEP;
		if (event->key.keysym.sym == SDLK_RIGHT) unicorn->deltaPos.x = POS_STEP;*/

		//otherwise
		if (event->key.keysym.sym == SDLK_RIGHT) player->speedX = UNICORN_STARTING_SPEED * 2;
		if (event->key.keysym.sym == SDLK_LEFT) player->speedX = UNICORN_STARTING_SPEED / 2;
		break;
	case SDL_KEYUP:
		if (event->key.keysym.sym == SDLK_UP) player->deltaPos.y = 0.0f;
		if (event->key.keysym.sym == SDLK_DOWN) player->deltaPos.y = 0.0f;

		//in case horizontal movement of the Unicorn on the screen is allowed
		/*if (event->key.keysym.sym == SDLK_LEFT) unicorn->deltaPos.x = 0.0f;
		if (event->key.keysym.sym == SDLK_RIGHT) unicorn->deltaPos.x = 0.0f;*/

		//otherwise
		if (event->key.keysym.sym == SDLK_RIGHT) player->speedX = UNICORN_STARTING_SPEED;
		if (event->key.keysym.sym == SDLK_LEFT) player->speedX = UNICORN_STARTING_SPEED;
		break;
	}
}

void DefaultControls(GameState *gameState, SDL_Event *event, Player *player)
{
	switch (event->type) {
	case SDL_KEYDOWN:
		if (event->key.keysym.sym == SDLK_z)
		{
			if (player->playerState != DASH) player->targetPlayerState = JUMP_UP;
		}
		if (event->key.keysym.sym == SDLK_x) player->targetPlayerState = DASH;
		break;
	case SDL_KEYUP:
		if (event->key.keysym.sym == SDLK_z)
		{
			if (player->playerState != DASH) player->targetPlayerState = FALL;
		}
		break;
	}
}

void SwitchControls(Controls *controlsMode, Player *player)
{
	if (*controlsMode == DEFAULT)
	{
		*controlsMode = NAIVE;
		player->speedX = UNICORN_STARTING_SPEED;
	}
	else
	{
		*controlsMode = DEFAULT;
		player->unicorn.position.x = UNICORN_X;
	}
}

void ManagePause(AppState *appState, Time *time)
{
	switch (*appState)
	{
	case PAUSED:
		*appState = CONTINUE;
		time->timeScale = 1.0f; //no time modifier
		break;
	default:
		*appState = PAUSED;
		time->timeScale = 0.0f; //time stops
	}
}

MenuOptions ChangeActiveOption(MenuOptions *activeOption, int val)
{
	int result = *activeOption + val;
	if (result < 0) result += MENU_OPTIONS_COUNT;
	if (result >= MENU_OPTIONS_COUNT) result -= MENU_OPTIONS_COUNT;
	return (MenuOptions)result;
}

void ChooseOption(App *app, GameState *gameState, SDL_Surface **surfaces)
{
	app->appState = CONTINUE;
	switch (app->activeOption)
	{
	case LV1:
		InitializeLevel(app, gameState, surfaces, &gameState->level, 0);
		break;
	case LV2:
		InitializeLevel(app, gameState, surfaces, &gameState->level, 1);
		break;
	case LV3:
		InitializeLevel(app, gameState, surfaces, &gameState->level, 2);
		break;
	case EXIT:
	default:
		app->appState = QUIT;
		break;
	}
}

void MenuHandlingEvents(App *app, GameState *gameState, SDL_Event *event, SDL_Surface **surfaces)
{
	while (SDL_PollEvent(event)) {
		switch (event->type) {
		case SDL_KEYDOWN:
			if (event->key.keysym.sym == SDLK_ESCAPE) app->appState = QUIT;
			else if (event->key.keysym.sym == SDLK_d) SwitchControls(&app->controlsMode, &app->player);
			else if (event->key.keysym.sym == SDLK_UP) app->activeOption = (ChangeActiveOption(&app->activeOption, -1));
			else if (event->key.keysym.sym == SDLK_DOWN) app->activeOption = (ChangeActiveOption(&app->activeOption, 1));
			else if (event->key.keysym.sym == SDLK_RIGHT) ChooseOption(app, gameState, surfaces);
			break;
		case SDL_QUIT:
			app->appState = QUIT;
			break;
		}
	}
}

void CallControlsModule(App *app, GameState *gameState, SDL_Event *event)
{
	switch (app->controlsMode)
	{
	case DEFAULT:
		DefaultControls(gameState, event, &app->player);
		break;
	case NAIVE:
		NaiveControls(event, &app->player);
		break;
	default:
		app->appState = QUIT;
		break;
	}
}

void HandligEvents(App *app, GameState *gameState, Time *time, SDL_Event *event)
{
	while (SDL_PollEvent(event)) {
		CallControlsModule(app, gameState, event);
		switch (event->type) {
		case SDL_KEYDOWN:
			if (event->key.keysym.sym == SDLK_ESCAPE) app->appState = QUIT;
			else if (event->key.keysym.sym == SDLK_d) SwitchControls(&app->controlsMode, &app->player);
			if (app->player.playerState != DEAD)
			{
				if (event->key.keysym.sym == SDLK_p) ManagePause(&app->appState, &gameState->time);
				else if (event->key.keysym.sym == SDLK_n) app->appState = RESET;
			}
			else
			{
				if (event->key.keysym.sym == SDLK_y) app->appState = RESET;
				else if (event->key.keysym.sym == SDLK_n) app->appState = MENU;
			}
			break;
		case SDL_QUIT:
			app->appState = QUIT;
			break;
		}
	}
}

#pragma endregion

#pragma region GameLogic

void ApplyGravity(Player *player, double deltaTime)
{
	player->speedY += GRAVITY * deltaTime;
}


void MoveLooping(GameObject* gameObjects, int n, double val, double jump)
{
	for (int gameObject_i = 0; gameObject_i < n; gameObject_i++)
	{
		double targetXpos = gameObjects[gameObject_i].position.x - val;
		if (targetXpos < -gameObjects[gameObject_i].dimensions.x / 2) targetXpos += jump;
		gameObjects[gameObject_i].position.x = targetXpos;
	}
}

void MoveLevelElements(Level *lv, Player *player, double deltaTime)
{
	MoveLooping(lv->sky, BACKGROUND_LOOP_ELEMENTS, SKY_SCROLL_SPEED * deltaTime, BACKGROUND_LOOP_ELEMENTS*BACKGROUND_WIDTH);
	MoveLooping(lv->clouds, BACKGROUND_LOOP_ELEMENTS, CLOUDS_SCROLL_SPEED * deltaTime, BACKGROUND_LOOP_ELEMENTS*BACKGROUND_WIDTH);
	for (int group_i = 0; group_i < LEVEL_LAYERS_COUNT; group_i++)
	{
		MoveLooping(lv->elemArrays[group_i], lv->elemsCount[group_i], player->speedX, lv->length);
	}
}

void DefaultStateMachine(App *app, GameState *gameState, Player *player)
{
	Accelerate(player, gameState->time.delta);
	switch (player->playerState)
	{
	case DASH:
		ManageDashing(player, gameState->time.delta);
		break;
	case RUN:
		if (!DetectGround(app, gameState, { 0.0f, GROUND_MARGIN_MAX }))
		{
			player->targetPlayerState = FALL;
		}
		break;
	case FALL:
		ApplyGravity(player, gameState->time.delta);
		MoveUnicornY(app, gameState, &player->unicorn, player->speedY, gameState->time.delta);
		break;
	case JUMP_UP:
		ManageJumping(player, gameState->time.delta);
		ApplyGravity(player, gameState->time.delta);
		MoveUnicornY(app, gameState, &player->unicorn, player->speedY, gameState->time.delta);
		break;
	default:
		break;
	}
}

void UpdateGameObjects(App *app, GameState *gameState, Player *player, SDL_Surface **surfaces, Level *lv)
{
	if (player->playerState == DEAD)
	{
		ApplyGravity(player, gameState->time.delta);
		MoveUnicornY(app, gameState, &player->unicorn, player->speedY, gameState->time.delta);
		return;
	}
	if (player->targetPlayerState != player->playerState) ChangePlayerState(player, player->targetPlayerState);

	if (app->controlsMode == DEFAULT)
	{
		DefaultStateMachine(app, gameState, player);
	}
	else
	{
		//in case horizontal movement of the Unicorn on the screen is allowed
		//MoveUnicornX(&gameState->unicorn, gameState->player.deltaPos.x, gameState->time.delta);
		MoveUnicornY(app, gameState, &player->unicorn, player->deltaPos.y, gameState->time.delta);
	}
	MoveLevelElements(lv, player, gameState->time.delta);
	if (DetectObstacleCollision(app, gameState) || DetectFrontTerrainCollision(app, gameState))
	{
		Die(app, player, gameState->time.worldTime);
	}
	DetectBonusCollision(app, gameState);
}

void CountScore(GameState *gameState, Player *player)
{
	if (player->playerState == DEAD) return;
	gameState->score += player->speedX * gameState->time.delta * SCORE_MULTIPLIER;
	if (gameState->score > gameState->nextMileStone)
	{
		gameState->nextMileStone += MILESTONE_DISTANCE;
		gameState->dolphinsScreenTime = DOLPHIN_SCREEN_TIME;
	}
}
#pragma endregion

void UpdateGameFrame(App *app, GameState *gameState, SDL_Surface **surfaces, SDL_Surface *screen, int *colors)
{
	//apply gamelogic
	UpdateGameObjects(app, gameState, &app->player, surfaces, &gameState->level);
	CountScore(gameState, &app->player);
	RenderBMPs(app, gameState, screen, surfaces);
}

void NextFrame(App *app, GameState *gameState, SDL_Event *event, SDL_Surface **surfaces, int *colors)
{
	ManageTime(&gameState->time);
	ManageFPS(&gameState->time);
	switch (app->appState) 
	{
	case CONTINUE:
		UpdateGameFrame(app, gameState, surfaces, app->scrrend.screen, colors);
		if (app->player.playerState != DEAD) RenderInfoText(app->scrrend.screen, colors, gameState, surfaces);
		else RenderGameOverText(app->scrrend.screen, colors, gameState, surfaces);
		break;
	case PAUSED:
		RenderPauseText(app->scrrend.screen, colors, gameState, surfaces);
		break;
	default:
		break;
	}
	
	UpdateScreen(&app->scrrend);
	// obs≥uga zdarzeÒ (o ile jakieú zasz≥y) / handling of events (if there were any)
	HandligEvents(app, gameState, &gameState->time, event);
	gameState->time.frames++;
}

void ManageMenu(App *app, GameState *gameState)
{
	RenderMainMenu(app, gameState, app->scrrend.screen, app->surfaces, app->colors);
	UpdateScreen(&app->scrrend);
	MenuHandlingEvents(app, gameState, &app->event, app->surfaces);
}

void RestartLevel(App *app, GameState *gameState, Player *player)
{
	ResetGameState(gameState);
	ResetPlayer(player);
	app->appState = CONTINUE;
}

bool InitializeApp(App *app)
{
	app->activeOption = LV1;
	app->appState = MENU;
	app->controlsMode = NAIVE;
	app->surfaces = (SDL_Surface **)malloc(SURFACES_COUNT * sizeof(SDL_Surface *));
	app->colors = (int *)malloc(COLORS_COUNT * sizeof(int));
	if (!SDL_Initialization(&app->scrrend, &app->scrrend.rc)) return false;
	AssignColors(app->colors, app->scrrend.screen);
	if (!LoadAllBMPs(app->surfaces, SURFACES_COUNT, &app->scrrend)) return false;
	SDL_SetColorKey(app->surfaces[CHARSET], true, 0x000000);
	InitializeResources(app, app->surfaces);
	return true;
}

void ApplicationLoop(App *app, GameState *gameState)
{
	while (app->appState != QUIT)
	{
		while (app->appState == MENU)
		{
			app->player.lives = MAX_LIVES;
			ManageMenu(app, gameState);
		}
		while (app->appState == CONTINUE || app->appState == PAUSED)
		{
			NextFrame(app, gameState, &app->event, app->surfaces, app->colors);
		}
		if (app->appState == RESET)
		{
			FreeLevelElements(&gameState->level);
			RestartLevel(app, gameState, &app->player);
			InitializeLevel(app, gameState, app->surfaces, &gameState->level, app->activeOption);
		}
	}
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	GameState gameState;
	App app;
	if(!InitializeApp(&app)) return -1;
	StartClock(&gameState.time);
	ApplicationLoop(&app, &gameState);
	Close(&app, &gameState);
	return 0;
}
