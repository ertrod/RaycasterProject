
#include <iostream>
#include <vector>
#include <algorithm>


#include "libs/SDL2/include/SDL.h"
#include "SDL2wrapper/include/SDL2wrapper.h"


struct vector2d
{
    double x, y;
};

struct vector2f 
{
    float x, y;
};

struct Sprite
{
    float x, y;
    int texture;
};

enum TILE_SIDE
{
    X, Y
};

struct Intersection
{
    int x, y;
    double fx, fy;
    double distance;
    TILE_SIDE side;
    float rayDistance;
};

struct Player
{
    vector2f pos;
    double angle;
    int fov;
};

const float PI = 3.1415;

// Screen and plane constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const int PLANE_WIDTH = 200;
const int PLANE_HEIGHT = 112;

const int HALF_PLANE_WIDTH = PLANE_WIDTH/2;

const int TILE_SIZE = 64;

const int PLAYER_FOV = 90;
const int PLAYER_HEIGHT = 32;

const int DISTANCE_TO_PLANE = PLANE_WIDTH / 2;

constexpr int MAP_TEXTURE_WIDTH = SCREEN_WIDTH * 2;
constexpr int MAP_TEXTURE_HEIGHT = SCREEN_HEIGHT * 2;

const int MAP_WIDTH = 16;
const int MAP_HEIGHT = 16;

// Framerate constants
int fpsCap = 200;   // maximum framerate
int tickRate = 120; // desired framerate

const int MAX_RAY_DISTANCE = 24;

const int PLAYER_SPEED = 10.;

const int MOUSE_MOTION_MULTIPLIER = 2.5;
const int MOUSE_MOTION_SPEED = 6.1;

// World map
int map[MAP_HEIGHT][MAP_WIDTH] = {
    {1, 2, 1, 2, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 4, 0, 4, 4, 4, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 3, 3, 3, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 3, 0, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 2, 1, 2, 1, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1},
};


const int SPRITE_COUNT = 7;

Sprite sprites[SPRITE_COUNT] = {
    {1.5, 1.5, 1},
    {6.5, 1.5, 1},
    {1.5, 6.5, 1},
    {10.5, 8.5, 1},
    {9.5, 9.5, 1},
    {10.5, 10.5, 1},
    {12.5, 11.5, 1}
};


Player player;

float DegToRad(float angle)
{
    return angle * PI / 180.;
}


void InitPlayer()
{
    player.pos = {2, 3};

    vector2f direction = { 1., -1. };
    player.angle = std::atan2(direction.y, direction.x);

    player.fov = DegToRad(PLAYER_FOV);
}

float depthBuffer[PLANE_WIDTH];

Intersection ClosestHitPoint(double rayAngle, vector2f startPos)
{
    Intersection i = { 0 };
    float x = 0;
    float y = 0;

    for (float ray = 0; ray < MAX_RAY_DISTANCE; ray += .01)
    {
        x = startPos.x + ray * std::cos(rayAngle);
        y = startPos.y + ray * std::sin(rayAngle);

        if (map[static_cast<int>(y)][static_cast<int>(x)])
        {
            float dist = ray * cos(rayAngle - player.angle);

            i.x = x;
            i.y = y;
            i.fx = x;
            i.fy = y;
            i.distance = dist;
            i.rayDistance = ray;
            break;
        }
    }
    return i;
}

void HandleMouseInput(int delta, SDL_Event e)
{
    if (e.type == SDL_MOUSEMOTION)
    {
        float angle = DegToRad(MOUSE_MOTION_MULTIPLIER * MOUSE_MOTION_SPEED) * (delta/1000.);

        if (e.motion.xrel < 0) // left
        {
            player.angle -= angle;
        }
        else if (e.motion.xrel > 0) // right
        {
            player.angle += angle;
        }
    }
}

void HandleKeyInput(int delta)
{
    const Uint8* keyStates = SDL_GetKeyboardState(nullptr);

    vector2f playerDirection = {
        static_cast<float>(cos(player.angle)), static_cast<float>(sin(player.angle))
    };

    float tickSpeed = PLAYER_SPEED/2 * (delta/1000.);

    if (keyStates[SDL_SCANCODE_W]) 
    {
        if (map[int(player.pos.y)][int(player.pos.x + playerDirection.x * tickSpeed)] == 0)
            player.pos.x += playerDirection.x * tickSpeed;
        if (map[int(player.pos.y + playerDirection.y * tickSpeed)][int(player.pos.x)] == 0)
            player.pos.y += playerDirection.y * tickSpeed;
    }
    if (keyStates[SDL_SCANCODE_S])
    {
        if (map[int(player.pos.y)][int(player.pos.x - playerDirection.x * tickSpeed)] == 0)
            player.pos.x -= playerDirection.x * tickSpeed;
        if (map[int(player.pos.y - playerDirection.y * tickSpeed)][int(player.pos.x)] == 0)
            player.pos.y -= playerDirection.y * tickSpeed;
    }
    if (keyStates[SDL_SCANCODE_A])
    {
        if (map[int(player.pos.y)][int(player.pos.x + playerDirection.y * tickSpeed)] == 0 &&
            map[int(player.pos.y - playerDirection.x * tickSpeed)][int(player.pos.x)] == 0)
        {
            player.pos.x += playerDirection.y * tickSpeed;
            player.pos.y -= playerDirection.x * tickSpeed;
        }
    }
    if (keyStates[SDL_SCANCODE_D])
    {
        if (map[int(player.pos.y)][int(player.pos.x - playerDirection.y * tickSpeed)] == 0 &&
            map[int(player.pos.y + playerDirection.x * tickSpeed)][int(player.pos.x)] == 0)
        {
            player.pos.x -= playerDirection.y * tickSpeed;
            player.pos.y += playerDirection.x * tickSpeed;
        }
    }
}

int DeltaTime(int previous, int offset)
{
    return (clock() - previous) + offset;
}


int main(int argc, char* argv[])
{
    InitPlayer();
    std::cout << player.angle << std::endl;
    try
    {
        sdl2::SDL sdl(SDL_INIT_VIDEO);
        sdl2::SDLTTF sdlttf;
        sdl2::Window window(
            "raycast", 
            SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT,
            0
        );
        sdl2::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        sdl2::Texture screen = sdl2::CreateTexture(renderer, 
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
            PLANE_WIDTH, PLANE_HEIGHT
        );

        sdl2::Texture screenMap = sdl2::CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
            MAP_TEXTURE_WIDTH, MAP_TEXTURE_HEIGHT
        );

        sdl2::Texture wolfTextures = sdl2::CreateTexture(renderer, "data/wolftextures.png");
        int floorTexture = 6 * TILE_SIZE;
        int ceilTexture = 7 * TILE_SIZE;

        sdl2::Texture entityTexture = sdl2::CreateTexture(renderer, "data/enemy.png");

        sdl2::Font font("data/fonts/Vera.ttf", 20);

        int rectWidth = PLANE_WIDTH / HALF_PLANE_WIDTH;
        
        // time variables
        int prevTime = clock();
        int offset = 0;

        // Sprites
        std::vector<std::pair<int, float>> entities(SPRITE_COUNT);

        // stick mouse to game window
        SDL_SetRelativeMouseMode(SDL_TRUE);

        float zBuffer[PLANE_WIDTH];

        SDL_Event e;
        bool quit = false;
        while (quit == false)
        {
            // get delta time
            int delta = DeltaTime(prevTime, offset);
            prevTime = clock();

            renderer.Target(screen);
            renderer.SetDrawColor(255, 255, 255);
            renderer.Clear();

            for (int i = 0; i <= PLANE_WIDTH; i++)
            {
                float angle = (player.angle - player.fov/2.) + player.fov/float(PLANE_WIDTH) * i;

                Intersection wall = ClosestHitPoint(angle, player.pos);
                zBuffer[i] = wall.distance;

                int sliceSize = int(PLANE_WIDTH/ wall.distance);
                int sliceX = i;
                int sliceY = PLANE_HEIGHT/2 - sliceSize/2;

                sdl2::Rect slice(sliceX, sliceY, 1, sliceSize);

                // set texture
                float x = wall.fx - floor(wall.fx+.5); // x and y contain (signed) fractional parts of hitx and hity,
                float y = wall.fy - floor(wall.fy+.5); // they vary between -0.5 and +0.5, and one of them is supposed to be very close to 0

                float tex = x*TILE_SIZE;

                if (std::abs(y) >= std::abs(x)) // we need to determine whether we hit a "vertical" or a "horizontal" wall (w.r.t the map)
                {
                    tex = y*TILE_SIZE;
                    wolfTextures.SetColorMod(128, 128, 128);
                }

                if (tex < 0) // do not forget x_texcoord can be negative, fix that
                    tex += TILE_SIZE;
                    

                tex += (map[wall.x][wall.y] * TILE_SIZE); // get proper texture according on what wall on map

                sdl2::Rect srcrect(static_cast<int>(tex), 0, 1, wolfTextures.Height());
                renderer.Copy(wolfTextures, srcrect, slice);

                wolfTextures.SetColorMod(255, 255, 255);
                
                // floor casting
                int px = slice.x;
                for (int py = slice.y+slice.h; py <= PLANE_HEIGHT; py++)
                {
                    float p = py - (PLANE_HEIGHT / 2);
                    float rowDist = ((float(DISTANCE_TO_PLANE) ) / (p)) / cos(angle - player.angle);

                    float floorX = (player.pos.x + cos(angle) * rowDist);
                    float floorY = (player.pos.y + sin(angle) * rowDist);

                    int cellX = static_cast<int>(floorX);
                    int cellY = static_cast<int>(floorY);

                    int ftx = (int(floorTexture + TILE_SIZE * (floorX - cellX)));
                    int fty = (int(TILE_SIZE * (floorY - cellY)));

                    int ctx = (int(ceilTexture + TILE_SIZE * (floorX - cellX)));

                    
                    sdl2::Rect srcf(ftx, fty, slice.w, 1);
                    sdl2::Rect dstf(slice.x, py, slice.w, 1);

                    int cy = PLANE_HEIGHT - py;

                    sdl2::Rect srcc(ctx, fty, slice.w, 1);
                    sdl2::Rect dstc(slice.x, cy, slice.w, 1);

                    wolfTextures.SetColorMod(128, 128, 128);
                    renderer.Copy(wolfTextures, srcc, dstc);
                    wolfTextures.SetColorMod(255, 255, 255);
                    renderer.Copy(wolfTextures, srcf, dstf);
                }
            }

            for (int i = 0; i < SPRITE_COUNT; i++)
            {
                entities[i].first = i;

                float xDist = player.pos.x - sprites[i].x;
                float yDist = player.pos.y - sprites[i].y;

                entities[i].second = sqrt((xDist*xDist) + (yDist*yDist));

            }

            renderer.Target(screen);
            std::sort(entities.begin(), entities.end(), [](auto &left, auto &right){
                return left.second > right.second;
            });

            
            for (int i = 0; i < SPRITE_COUNT; i++)
            {

                float spriteDir = atan2(sprites[entities[i].first].y - player.pos.y, sprites[entities[i].first].x - player.pos.x);

                
                while ((spriteDir - player.angle) > PI) spriteDir -= 2*PI;
                while ((spriteDir - player.angle) < -PI) spriteDir += 2*PI;

                
                entities[i].second *= cos(spriteDir - player.angle);
                {
                    int spriteHeight = PLANE_WIDTH / entities[i].second;

                    int spriteScreenY = (PLANE_HEIGHT/2 - spriteHeight/2);
                    float spriteScreenX = (spriteDir - player.angle) * (float(PLANE_WIDTH) / player.fov) + float(PLANE_WIDTH/2);


                    int drawStartX = spriteScreenX - spriteHeight/2;
                    int drawEndX = drawStartX + spriteHeight;

                    int texWidth = spriteHeight;
                    float texStepX = entityTexture.Width() / static_cast<float>(texWidth);

                    int texStartX = 0;
                    int texEndX = TILE_SIZE;

                    int screenStartX = drawStartX;
                    if ((drawStartX >= 0 && drawStartX <= PLANE_WIDTH) || (drawEndX >= 0 && drawEndX <= PLANE_WIDTH))
                    {
                        if (drawStartX < 0)
                        {
                            texStartX = (0 - screenStartX) * texStepX;
                            screenStartX = 0;
                        }
                        int screenEndX = drawEndX;
                        if (drawEndX > PLANE_WIDTH)
                        {
                            texEndX = (drawEndX - PLANE_WIDTH) * texStepX; 
                            screenEndX = PLANE_WIDTH;
                        }

                        if (screenStartX < PLANE_WIDTH && screenEndX <= PLANE_WIDTH)
                            entities[i].second *= cos(spriteDir - player.angle);
                        
                        float texX = texStartX;
                        for (int j = screenStartX; j < screenEndX; j++)
                        {
                            if (screenStartX < PLANE_WIDTH && screenEndX <= PLANE_WIDTH && zBuffer[j] > entities[i].second)
                            {
                                sdl2::Rect spriteSrc(texX, 0, 1, entityTexture.Height());
                                sdl2::Rect spriteDst(j, spriteScreenY, 1, spriteHeight);
                                renderer.Copy(entityTexture, spriteSrc, spriteDst);

                            }
                            texX += texStepX;
                        }
                    }
                }
            }

            while(SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }

                if (e.type == SDL_KEYDOWN)
                {
                    switch(e.key.keysym.sym)
                    {

                        case SDLK_q:
                            quit = true;
                            break;
                    }
                }

                HandleMouseInput(delta, e);
            }

            HandleKeyInput(delta);

            renderer.Target();

            renderer.Copy(screen, std::nullopt, sdl2::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0, std::nullopt);

            renderer.Present();

            offset = delta % (1000 / tickRate);
        }
    }
    catch (sdl2::SDLException e)
    {
        std::cerr << e.what() << std::endl;
    }


    return 0;
}
