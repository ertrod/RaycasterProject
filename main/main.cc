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

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int WALL_HEIGHT = 64;
const int PLAYER_HEIGHT = 32;

const int TILE_SIZE = 64;

const int PLANE_WIDTH = SCREEN_WIDTH / 2;
const int PLANE_HEIGHT = SCREEN_HEIGHT / 2;

const int FOV = 60;

const int DISTANCE_TO_PLANE = (PLANE_WIDTH / 2) / std::tan(DegToRad(FOV/2));

const double PLAYER_VELOCITY = 0.05; 
Player player;

void InitPlayer()
{
    player.pos = {1.5, 1.5};
    player.direction = {-1, 1};
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
        if (map[mapX][mapY])
        {
            break;
        }
    }
    return Intersection{mapX, mapY, distance, side};
}

double DistanceToPoint(vector2f startPoint, vector2f endPoint)
{
    double squaredDistance = std::pow(startPoint.x - endPoint.x, 2) + std::pow(startPoint.y - endPoint.y, 2);
    return std::sqrt(squaredDistance);
}

int main(int args, char* argv[])
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
            SCREEN_WIDTH, SCREEN_HEIGHT,
            0
        );
        sdl2::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        sdl2::Font font("main/data/Vera.ttf", 20);

        sdl2::Surface text_surface = font.RenderText_Solid("00.00", {255, 255, 255});
        sdl2::Texture text = CreateTexture(renderer, text_surface);
        int rotAngle = 3;
        double halfHeigth = tan(DegToRad(FOV) / 2);
        double halfWidth = (double(PLANE_WIDTH) / double(PLANE_HEIGHT)) * halfHeigth;
        int rectWidth = SCREEN_WIDTH / PLANE_WIDTH;

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
                if (e.type == SDL_KEYDOWN)
                {
                    switch(e.key.keysym.sym)
                    {
                        case SDLK_w:
                            player.pos.x += player.direction.x * PLAYER_VELOCITY;
                            player.pos.y += player.direction.y * PLAYER_VELOCITY;
                            break;
                        case SDLK_s:
                            player.pos.x -= player.direction.x * PLAYER_VELOCITY;
                            player.pos.y -= player.direction.y * PLAYER_VELOCITY;
                            break;
                        case SDLK_d:
                        {
                            int opposite_angle = 360 - rotAngle;
                            player.direction = {
                                (player.direction.x * CosTable[opposite_angle] - player.direction.y * SinTable[opposite_angle]),
                                (player.direction.x * SinTable[opposite_angle] + player.direction.y * CosTable[opposite_angle])
                            };
                            break;
                        }
                        case SDLK_a:
                        {
                            player.direction = {
                                (player.direction.x * CosTable[rotAngle] - player.direction.y * SinTable[rotAngle]),
                                (player.direction.x * SinTable[rotAngle] + player.direction.y * CosTable[rotAngle])
                            };
                            break;
                        }
                    }
                }
            }

            renderer.SetDrawColor(50, 50, 100);
            renderer.Clear();

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

            for (int i = 0; i < PLANE_WIDTH; i++)
            {
                double ray_angle = angle + (FOV/2) - (i * FOV/PLANE_WIDTH);
                double offset = ((i * 2.) / (PLANE_WIDTH - 1.) - 1.);

                float addX = right.x * offset;
                float addY = right.y * offset;
                vector2f ray_dir = {
                    static_cast<float>(forward.x + addX),
                    static_cast<float>(forward.y + addY)
                };

                Intersection wall = ClosestHitPoint(ray_dir, player.pos);
                int slice_size = WALL_HEIGHT / (wall.distance*TILE_SIZE) * DISTANCE_TO_PLANE;

                int start_rect_x = i * rectWidth;
                int start_rect_y = (SCREEN_HEIGHT/2 - slice_size/2); 
                sdl2::Rect rect(start_rect_x, start_rect_y, rectWidth, slice_size);


                switch (map[wall.x][wall.y])
                {
                    case 1:
                        renderer.SetDrawColor(255, 255, 255);
                        break;
                    case 2:
                        renderer.SetDrawColor(255, 255, 0);
                        break;
                    case 3:
                        renderer.SetDrawColor(255, 0, 0);
                        break;
                    case 4:
                        renderer.SetDrawColor(0, 255, 0);
                        break;
                }

                if (wall.side == Y)
                {
                    sdl2::Color c = renderer.GetDrawColor();
                    renderer.SetDrawColor(c.r/2, c.g/2, c.b/2);
                }
                
                renderer.FillRect(rect);

                text.Update(std::nullopt, font.RenderText_Solid(std::to_string(wall.distance), {255, 255, 255}));
            }

            // SDL_Delay(100);

            renderer.Copy(text, std::nullopt, {0, 0});


            renderer.Present();
        }
    }
    catch (sdl2::SDLException e)
    {
        std::cerr << e.SDLFunction() << " " << e.SDLError() << std::endl;
    }

    return 0;
}