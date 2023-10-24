#include <iostream>

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


int map[7][6] = {
    {1, 2, 1, 2, 1, 1},
    {1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 2},
    {2, 0, 0, 0, 0, 1},
    {1, 0, 3, 2, 0, 2},
    {1, 0, 0, 3, 0, 1},
    {1, 2, 1, 1, 2, 1},
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

Intersection ClosestHitPoint(vector2f ray_dir, vector2f start_pos)
{
    int mapx = static_cast<int>(start_pos.x);
    int mapy = static_cast<int>(start_pos.y);
    
    double dirx = ray_dir.x;
    double diry = ray_dir.y;

    double deltax = (dirx == 0) ? 1e30 : std::abs(1 / dirx);
    double deltay = (diry == 0) ? 1e30 : std::abs(1 / diry);

    double rayx = 0;
    double rayy = 0;

    double distance = 0.f;

    int sx = 0;
    int sy = 0;

    if (dirx > 0)
    {
        sx = 1;
        rayx = (double(mapx) + 1.f - start_pos.x) * deltax;
    }
    else
    {
        sx = -1;
        rayx = (start_pos.x - mapx) * deltax;
    }
    if (diry > 0)
    {
        sy = 1;
        rayy = (double(mapy) + 1.f - start_pos.y) * deltay;
    }
    else
    {
        sy = -1;
        rayy = (start_pos.y - mapy) * deltay;
    } 
    TILE_SIDE side;
    while (distance < MAX_RAY_LENGTH)
    {
        bool goX = rayx < rayy;
        if (goX)
        {
            mapx += sx;
            distance = rayx;
            rayx += deltax;
            side = X;
        }
        else
        {
            mapy += sy;
            distance = rayy;
            rayy += deltay;
            side = Y;
        }
        if (map[mapx][mapy])
        {
            break;
        }
    }
    return Intersection{mapx, mapy, distance, side};
}

double DistanceToPoint(vector2f start_point, vector2f end_point)
{
    double squared_distance = std::pow(start_point.x - end_point.x, 2) + std::pow(start_point.y - end_point.y, 2);
    return std::sqrt(squared_distance);
}


int main(int args, char* argv[])
{
    InitPlayer();
    InitTables();
    try
    {
        sdl2::SDL sdl(SDL_INIT_VIDEO);
        sdl2::Window window(
            "raycast", 
            SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT,
            0
        );
        sdl2::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        int rot_angle = 3;
        double halfHeigth = tan(DegToRad(FOV) / 2);
        double halfWidth = (double(PLANE_WIDTH) / double(PLANE_HEIGHT)) * halfHeigth;
        int rect_width = SCREEN_WIDTH / PLANE_WIDTH;

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
                            int opposite_angle = 360 - rot_angle;
                            player.direction = {
                                (player.direction.x * CosTable[opposite_angle] - player.direction.y * SinTable[opposite_angle]),
                                (player.direction.x * SinTable[opposite_angle] + player.direction.y * CosTable[opposite_angle])
                            };
                            break;
                        }
                        case SDLK_a:
                        {
                            player.direction = {
                                (player.direction.x * CosTable[rot_angle] - player.direction.y * SinTable[rot_angle]),
                                (player.direction.x * SinTable[rot_angle] + player.direction.y * CosTable[rot_angle])
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
                float add_x = right.x * offset;
                float add_y = right.y * offset;
                vector2f ray_dir = {
                    static_cast<float>(forward.x + add_x),
                    static_cast<float>(forward.y + add_y)
                };

                Intersection wall = ClosestHitPoint(ray_dir, player.pos);
                int slice_size = WALL_HEIGHT / (wall.distance*TILE_SIZE) * DISTANCE_TO_PLANE;

                int start_rect_x = i * rect_width;
                int start_rect_y = (SCREEN_HEIGHT/2 - slice_size/2); 
                sdl2::Rect rect(start_rect_x, start_rect_y, rect_width, slice_size);


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
                }

                if (wall.side == Y)
                {
                    sdl2::Color c = renderer.GetDrawColor();
                    renderer.SetDrawColor(c.r/2, c.g/2, c.b/2);
                }
                
                renderer.FillRect(rect);
            }

            // SDL_Delay(100);
            renderer.Present();
        }
    }
    catch (sdl2::SDLException e)
    {
        std::cerr << e.SDLFunction() << e.SDLError() << std::endl;
    }

    return 0;
}