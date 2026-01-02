#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "SDL3/SDL.h"

#include "structures.h"
#include "render.h"
#include "penguin.h"
// #include "cube.h"

int width = 800;
int height = 800;
int FPS = 60;




void DrawLine(SDL_Renderer* renderer, ScreenPoint p1, ScreenPoint p2)
{
    SDL_SetRenderDrawColor(renderer, 255, 0, 80, 255);
    SDL_RenderLine(renderer, p1.x, p1.y, p2.x, p2.y);
}



int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS))
    {
        printf("Error init");
        return 0;
    }
    printf("SDL Init Success\n");

    WindowInfo program = {800, 800, 60};

    
    SDL_Window* window = SDL_CreateWindow("Program", program.width, program.height, SDL_WINDOW_RESIZABLE);
    printf("SDL window Success\n");
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    printf("SDL renderer Success\n");


    // Main Loop
    int quit = 0;
    SDL_Event event;


    float dz = 1;
    float angle = 0;


    // Creating an object
    // -----------------------------------------------------------------------
    printf("Creating object\n");
    // printf("Getting Mesh counts\n");
    int vertexCount = sizeof(verts) / sizeof(verts[0]);
    int faceCount = sizeof(faces) / sizeof(faces[0]);
    
    // printf("Creating Transform\n");
    Transform obj1Transform = {0.0, 0.0, 1.0};
    // printf("Creating Mesh\n");
    Mesh* obj1Mesh = malloc(sizeof(Mesh) + vertexCount * sizeof(Point));
    // printf("Getting vertexCount");
    obj1Mesh->vertexCount = vertexCount;
    // printf("Getting facesCount");
    obj1Mesh->facesCount = faceCount;
    // printf("Copying faces\n");
    obj1Mesh->faces = malloc(faceCount * sizeof *obj1Mesh->faces);
    memcpy(obj1Mesh->faces, faces, faceCount * sizeof(*obj1Mesh->faces));
    // printf("Creating vertices\n");
    memcpy(obj1Mesh->vertices, verts, vertexCount * sizeof(Point));
    // printf("Creating and setting object\n");
    Object obj1;
    // printf("setting Transform object\n");
    obj1.transform = obj1Transform;
    // printf("setting Mesh object\n");
    obj1.mesh = obj1Mesh;
    // printf("Object done\n");
    // -----------------------------------------------------------------------
    
    const float dt = 1.0/program.FPS;
    while (quit == 0)
    {
        // printf("Polling events\n");
        while (SDL_PollEvent(&event))
        {
            // printf("Checking event type\n");
            if (event.type == SDL_EVENT_QUIT)
                quit = 1;
            
            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                SDL_GetWindowSizeInPixels(window, &program.width, &program.height);
            }
        }

        // Set Background
        // printf("Setting background color\n");
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderClear(renderer);


        // printf("Making animation variables\n");
        // Render Point Animation
        angle = 3.14159265 / 128;

        // printf("Rotating vertices\n");
        for (int i = 0; i < obj1.mesh->vertexCount; i++)
        {
            // Rotating object around an axis
            rotate_xz(obj1Mesh, &obj1Mesh->vertices[i], -angle);
            // rotate_xy(obj1Mesh, &obj1Mesh->vertices[i], -angle);
            // rotate_yz(obj1Mesh, &obj1Mesh->vertices[i], angle);

            // Translate Object in the z direction
            translateObjectZ(&obj1, 0.00002);


            // Uncomment if you want to render the individual points
            // RenderPoint(renderer, Screen(Project(obj1, obj1->vertices[i])));
        }

        // printf("Drawing faces\n");
        for (int i = 0; i < obj1.mesh->facesCount; ++i)
        {
            int* row = obj1.mesh->faces[i];

            // int length = sizeof(row) / sizeof(row[0]);
            for (int j = 0; j < 3; ++j)
            {
                int a = row[j];
                int b = row[(j+1)%3];
                DrawLine(renderer, Screen(Project(&obj1.transform, obj1.mesh->vertices[a]), program), Screen(Project(&obj1.transform, obj1.mesh->vertices[b]), program));
            }
        }


        // Needed to keep image
        SDL_RenderPresent(renderer);

        SDL_Delay(1000/program.FPS);
    }


    // Exiting functions
    printf("Quitting SDL\n");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();


    return 0;
}


/*
Compile command:
gcc main.c -o Program (-Llib -Iinclude) -lSDL3
*/