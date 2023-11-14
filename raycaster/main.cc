#include <iostream>

#define SDL2WRAPPER_FONT
#define SDL2WRAPPER_IMAGE

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
struct vector2i
{
    int x, y;
};

struct Player
{
    vector2f pos;
    vector2f direction;
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
};

const double PI = std::acos(-1);

double RadToDeg(double rad)
{
    return rad * (180/PI);
}
double DegToRad(double deg)
{
    return deg * (PI/180);
}

float SinTable[360];
float CosTable[360];


void InitTables()
{
    for (int i = 0; i < 360; i++)
    {
        float rad = DegToRad(i) + (0.0001);
        SinTable[i] = sin(rad);
        CosTable[i] = cos(rad);
    }
}

const int MAP_WIDTH = 6;
const int MAP_HEIGHT = 6;


int map[16][16] = {
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

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 225;

const int WALL_HEIGHT = 64;
const int PLAYER_HEIGHT = 32;

const int TILE_SIZE = 64;

const int PLANE_WIDTH = SCREEN_WIDTH/2;
const int PLANE_HEIGHT = SCREEN_HEIGHT;

const int FOV = 60;

const int DISTANCE_TO_PLANE = (PLANE_WIDTH / 2) / std::tan(DegToRad(FOV/2));

const double PLAYER_VELOCITY = 0.1;
Player player;

void InitPlayer()
{
    player.pos = {1.5, 1.5};
    player.direction = {1, -1};
}

const int MAX_RAY_LENGTH = 24;

Intersection ClosestHitPoint(vector2f rayDir, vector2f startPos)
{
    int mapX = static_cast<int>(startPos.x);
    int mapY = static_cast<int>(startPos.y);
    
    double dirX = rayDir.x;
    double dirY = rayDir.y;

    double deltaX = (dirX == 0) ? 1e30 : std::abs(1 / dirX);
    double deltaY = (dirY == 0) ? 1e30 : std::abs(1 / dirY);

    double rayX = 0;
    double rayY = 0;

    double distance = 0.f;

    int stepX = 0;
    int stepY = 0;

    if (dirX > 0)
    {
        stepX = 1;
        rayX = (double(mapX) + 1.f - startPos.x) * deltaX;
    }
    else
    {
        stepX = -1;
        rayX = (startPos.x - mapX) * deltaX;
    }
    if (dirY > 0)
    {
        stepY = 1;
        rayY = (double(mapY) + 1.f - startPos.y) * deltaY;
    }
    else
    {
        stepY = -1;
        rayY = (startPos.y - mapY) * deltaY;
    } 

    TILE_SIDE side;
    while (distance < MAX_RAY_LENGTH)
    {
        bool goX = rayX < rayY;
        if (goX)
        {
            mapX += stepX;
            distance = rayX;
            rayX += deltaX;
            side = X;
        }
        else
        {
            mapY += stepY;
            distance = rayY;
            rayY += deltaY;
            side = Y;
        }
        // for correct "3d" visualisation x and y need to be swapped
        if (map[mapX][mapY])
        {
            break;
        }
    }

    return Intersection{mapX, mapY, startPos.x + distance * dirX, startPos.y + distance * dirY, distance, side};
}




double DistanceToPoint(vector2f startPoint, vector2f endPoint)
{
    double squaredDistance = std::pow(startPoint.x - endPoint.x, 2) + std::pow(startPoint.y - endPoint.y, 2);
    return std::sqrt(squaredDistance);
}

int main(int argc, char* argv[])
{
    InitPlayer();
    InitTables();
    try
    {
        sdl2::SDL sdl(SDL_INIT_VIDEO);
        sdl2::SDLTTF sdlttf;
        sdl2::Window window(
            "raycast", 
            SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            0
        );
        sdl2::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        sdl2::Texture screen = sdl2::CreateTexture(renderer, 
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
            SCREEN_WIDTH, SCREEN_HEIGHT
        );

        sdl2::Texture screenMap = sdl2::CreateTexture(renderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
            SCREEN_HEIGHT, SCREEN_WIDTH
        );

        sdl2::Surface wolf("data/wolftextures.png");
        sdl2::Texture wolfTextures = sdl2::CreateTexture(renderer, 
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
            wolf.Width(), wolf.Height()
        );
        wolfTextures.Update(std::nullopt, wolf);
        wolfTextures.BlendMode(SDL_BLENDMODE_BLEND);

        sdl2::Font font("data/fonts/Vera.ttf", 20);
        sdl2::Surface text_surface = font.RenderText_Solid("00.00", {255, 255, 255});
        sdl2::Texture text = CreateTexture(renderer, text_surface);

        int rotAngle = 2;
        int rotationSpeed = 4;
        double halfHeigth = tan(DegToRad(FOV) / 2);
        double halfWidth = (double(PLANE_WIDTH) / double(PLANE_HEIGHT)) * halfHeigth;
        int rectWidth = SCREEN_WIDTH / PLANE_WIDTH;

        int floorTextureX = 6 * TILE_SIZE; // 7 texture from wolftextures.png
        int ceilTextureX = 7 * TILE_SIZE; // 8 texture from wolftextures.png
        int halfScreen = PLANE_HEIGHT / 2;

        int fogMaxDistance = 7;
        bool fogEnabled = true;
        int fogColorStep = 255 / fogMaxDistance;
        int minSliceSizeWithFog = WALL_HEIGHT / static_cast<float>(fogMaxDistance * TILE_SIZE) * DISTANCE_TO_PLANE;
        int minYWithFogToBlur = screen.Height()/2 + minSliceSizeWithFog/2 - 1;

        float cosAddition = (1 - CosTable[1]) / 10;
        float sinAddition = (SinTable[1]) / 10;
        
        SDL_SetRelativeMouseMode(SDL_TRUE);

        SDL_Event e;
        bool quit = false;
        while (quit == false)
        {
            while(SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                }
                if (e.type == SDL_MOUSEMOTION)
                {
                    // std::cout << e.motion.xrel << std::endl;
                    // int angle = RadToDeg(atan2(player.direction.y, player.direction.x)) + e.motion.xrel % 360;
                    int angle = 1;
                    if (e.motion.xrel < 0) // left
                    {
                        float cosRotation = CosTable[angle]; // + (cosAddition * (10 - rotationSpeed));
                        float sinRotation = SinTable[angle]; // - (sinAddition * (10 - rotationSpeed));
                        player.direction = {
                            (player.direction.x * cosRotation - player.direction.y * sinRotation),
                            (player.direction.x * sinRotation + player.direction.y * cosRotation)
                        };
                    }
                    else if (e.motion.xrel > 0) // right
                    {
                        float cosRotation = CosTable[360 - angle]; // - (cosAddition * (10 - rotationSpeed));
                        float sinRotation = SinTable[360 - angle]; // + (sinAddition * (10 - rotationSpeed));
                        player.direction = {
                            (player.direction.x * cosRotation - player.direction.y * sinRotation),
                            (player.direction.x * sinRotation + player.direction.y * cosRotation)
                        };
                    }
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
            }
            const Uint8* keyStates = SDL_GetKeyboardState(nullptr);
            if (keyStates[SDL_SCANCODE_W]) 
            {
                player.pos.x += player.direction.x * PLAYER_VELOCITY;
                player.pos.y += player.direction.y * PLAYER_VELOCITY;
            }
            if (keyStates[SDL_SCANCODE_S])
            {
                player.pos.x -= player.direction.x * PLAYER_VELOCITY;
                player.pos.y -= player.direction.y * PLAYER_VELOCITY;
            }
            if (keyStates[SDL_SCANCODE_D])
            {
                player.pos.x += player.direction.y * PLAYER_VELOCITY;
                player.pos.y -= player.direction.x * PLAYER_VELOCITY;
            }
            if (keyStates[SDL_SCANCODE_A])
            {
                player.pos.x -= player.direction.y * PLAYER_VELOCITY;
                player.pos.y += player.direction.x * PLAYER_VELOCITY;
            }

            renderer.Target(screen);

            renderer.SetDrawColor(50, 50, 100);
            renderer.Clear();

            renderer.Target(screenMap);
            renderer.Clear();
            renderer.Target();


            renderer.SetDrawColor(255, 255, 255);
            int angle = RadToDeg(atan2(player.direction.y, player.direction.x));
            if (angle < 0)
                angle += 360;
            else if (angle > 360)
                angle %= 360;

            vector2f forward = {
                CosTable[angle], SinTable[angle]
            };
            vector2f right = {
                forward.y, -forward.x
            };

            // 2d raycast
            renderer.Target(screenMap);
            int playerXoffset = SCREEN_HEIGHT / 2;
            int playerYoffset = SCREEN_WIDTH / 2;
            int playerX = static_cast<int>(player.pos.x + playerXoffset);
            int playerY = static_cast<int>(player.pos.y + playerYoffset);
            int playerW = 20;
            int playerH = 20;
            renderer.SetDrawColor(255, 255, 255);

            // draw player
            sdl2::Rect player_rect = sdl2::Rect::FromCenter(
                playerX, playerY,
                playerW, playerH
            );
            renderer.FillRect(player_rect);
            // draw player direction;
            renderer.SetDrawColor(255, 0, 0);
            renderer.DrawLine(playerX, playerY, 
                playerX + player.direction.x * playerW,
                playerY + player.direction.y * playerH
            );


            for (int i = 0; i < PLANE_WIDTH; i++)
            {
                // double ray_angle = angle + (FOV/2) - (i * FOV/PLANE_WIDTH);
                double offset = ((i * 2.) / (PLANE_WIDTH - 1.) - 1.);

                float addX = right.x * offset;
                float addY = right.y * offset;
                vector2f ray_dir = {
                    static_cast<float>(forward.x + addX),
                    static_cast<float>(forward.y + addY)
                };
                int beta = RadToDeg(atan2(ray_dir.y, ray_dir.x));

                // wall casting
                Intersection wall = ClosestHitPoint(ray_dir, player.pos);


                // 2d raycast
                renderer.Target(screenMap);
                renderer.SetDrawColor(255, 255, 153);
                int rayX = playerX + ray_dir.x * wall.distance * TILE_SIZE;
                int rayY = playerY + ray_dir.y * wall.distance * TILE_SIZE;
                sdl2::Rect ray_rect = sdl2::Rect::FromCenter(
                    rayX, rayY, 2, 2
                );
                renderer.DrawRect(ray_rect);

                // 3d raycast
                renderer.Target(screen);

                int slice_size = 0;
                if (fogEnabled)
                {
                    int ci = 255 - fogColorStep * wall.distance;
                    if (wall.distance < fogMaxDistance)
                    {
                        slice_size = WALL_HEIGHT / (wall.distance * TILE_SIZE) * DISTANCE_TO_PLANE;
                    }
                    else
                    {
                        slice_size = WALL_HEIGHT / static_cast<float>(fogMaxDistance * TILE_SIZE) * DISTANCE_TO_PLANE;
                        ci = 0;
                    }
                    
                    // set color intensity
                    wolfTextures.SetColorMod(ci, ci, ci);
                }
                else
                {
                    slice_size = WALL_HEIGHT / (wall.distance*TILE_SIZE) * DISTANCE_TO_PLANE;
                }

                int start_rect_x = i * rectWidth;
                int start_rect_y = (SCREEN_HEIGHT/2 - slice_size/2);

                sdl2::Rect rect(start_rect_x, start_rect_y, rectWidth, slice_size);
                
                float wallX = 0.;
                float wy = wall.fy - std::floor(wall.fy);
                float wx = wall.fx - std::floor(wall.fx);
                if (wall.side == X)
                {
                    // wolfTextures.SetColorMod(128, 128, 128);
                    wy *= TILE_SIZE;
                    wallX = wy;
                }
                else
                {
                    // wolfTextures.SetColorMod(255, 255, 255);
                    wx *= TILE_SIZE;
                    wallX = wx;
                }
                if (i == PLANE_WIDTH/2)
                {
                    text.Update(std::nullopt, font.RenderText_Solid(std::to_string(wall.distance), {255, 255, 255}));
                }

                wallX += map[wall.x][wall.y] * TILE_SIZE - TILE_SIZE; // get proper texture according on what wall
                sdl2::Rect srcrect(static_cast<int>(wallX), 0, 1, wolfTextures.Height());
                renderer.Copy(wolfTextures, srcrect, rect);
                wolfTextures.SetColorMod(255, 255, 255);

                // floor casting
                int px = rect.x;
                
                int fcFogBlurRangeMax = minYWithFogToBlur + 100/static_cast<int>(fogMaxDistance);

                float fcFogColorStep = 255. / (fcFogBlurRangeMax - minYWithFogToBlur);

                for (int py = rect.y + rect.h; py < screen.Height(); py++)
                {
                    int p = py - (screen.Height() / 2);
                    float rowDist = static_cast<float>(DISTANCE_TO_PLANE) * static_cast<float>(PLAYER_HEIGHT) / static_cast<float>(p);

                    float floorX = player.pos.x + ray_dir.x * rowDist / TILE_SIZE;
                    float floorY = player.pos.y + ray_dir.y * rowDist / TILE_SIZE;
                    
                    int cellX = static_cast<int>(floorX);
                    int cellY = static_cast<int>(floorY);

                    int ftx = (int(floorTextureX + TILE_SIZE * (floorX - cellX)));
                    int fty = (int(TILE_SIZE * (floorY - cellY)));

                    int ctx = (int(ceilTextureX + TILE_SIZE * (floorX - cellX)));

                    if (fogEnabled)
                    {
                        int color = 255;
                        if (py >= minYWithFogToBlur && py <= fcFogBlurRangeMax)
                        {
                            color = fcFogColorStep * (py - minYWithFogToBlur);
                        }
                        wolfTextures.SetColorMod(color, color, color);
                    }
                    
                    sdl2::Rect srcf(ftx, fty, rect.w, 1);
                    sdl2::Rect dstf(rect.x, py, rect.w, 1);

                    int cy = screen.Height() - py;

                    sdl2::Rect srcc(ctx, fty, rect.w, 1);
                    sdl2::Rect dstc(rect.x, cy, rect.w, 1);

                    // wolfTextures.SetColorMod(128, 128, 128);
                    renderer.Copy(wolfTextures, srcc, dstc);
                    // wolfTextures.SetColorMod(255, 255, 255);
                    renderer.Copy(wolfTextures, srcf, dstf);
                }
            }
            wolfTextures.SetColorMod(255, 255, 255);

            renderer.Target();

            renderer.Copy(screen, std::nullopt, sdl2::Rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT), 0, std::nullopt);
            // renderer.Copy(floorCeil, std::nullopt, sdl2::Rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT), 0, std::nullopt);

            
            int mapWidth = WINDOW_WIDTH / 6;
            int mapHeight = WINDOW_HEIGHT / 6;
            sdl2::Rect playerRect = sdl2::Rect::FromCenter(screenMap.Width()/2, screenMap.Height()/2, screenMap.Width()/2, screenMap.Height()/2);
            renderer.Copy(screenMap, 
                playerRect,
                sdl2::Rect(0, WINDOW_WIDTH - mapHeight, mapWidth, mapHeight),
                angle+91, 
                sdl2::Point(mapWidth/2, mapHeight/2),
                SDL_FLIP_HORIZONTAL
            );
            renderer.Target();

            renderer.Copy(text, std::nullopt, {0, 0});

            renderer.Present();
        }
    }
    catch (sdl2::SDLException e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}