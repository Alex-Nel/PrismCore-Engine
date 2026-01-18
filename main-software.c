#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "SDL3/SDL.h"

#include "structures.h"
#include "softwareRender.h"
#include "penguin.h"
#include "cube.h"



Object* GlobalObjects = NULL;
int GlobalObjectCount = 0;
int GlobalObjectCapacity = 0;

Ray* GlobalRays = NULL;
int GlobalRayCount = 0;
int GlobalRayCapacity = 0;



void AddGlobalObject(Object obj)
{
    if (GlobalObjectCount >= GlobalObjectCapacity)
    {
        // Increase capacity
        if (GlobalObjectCapacity == 0)
            GlobalObjectCapacity = 2;
        else
            GlobalObjectCapacity *= 2;
        
        GlobalObjects = realloc(GlobalObjects, sizeof(Object) * GlobalObjectCapacity);
    }
    
    GlobalObjects[GlobalObjectCount++] = obj;
}

void AddRay(Ray r)
{
    if (GlobalRayCount >= GlobalRayCapacity)
    {
        // Increase capacity
        if (GlobalRayCapacity == 0)
            GlobalRayCapacity = 2;
        else
            GlobalRayCapacity *= 2;
        
        GlobalRays = realloc(GlobalRays, sizeof(Ray) * GlobalRayCapacity);
    }

    GlobalRays[GlobalRayCount++] = r;
}







int main(int argc, char *argv[])
{
    // SDL initialization
    if (!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS))
    {
        printf("Error init");
        return 0;
    }
    printf("SDL Init Success\n");

    // Window information, width, height, and FPS
    WindowInfo program = {800, 800, 120};
    
    // SDL Window and Renderer creation
    SDL_Window* window = SDL_CreateWindow("PrismCore", program.width, program.height, SDL_WINDOW_RESIZABLE);
    printf("SDL window Success\n");

    ///////// GPU renderer
    // SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    // SDL_Surface* windowSurface = NULL;
    // SDL_Surface* surface = NULL;

    ///////// Software renderer
    SDL_Surface* surface = SDL_CreateSurface(program.width, program.height, SDL_PIXELFORMAT_RGBA8888);
    SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
    SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(surface);


    printf("Renderer: %s\n", SDL_GetRendererName(renderer));
    printf("SDL renderer Success\n");


    


    Scene testScene = {0};

    // Creating an object
    // -----------------------------------------------------------------------
    printf("Creating object\n");
    int vertexCount = sizeof(verts) / sizeof(verts[0]);
    int faceCount = sizeof(faces) / sizeof(faces[0]);

    Object obj1 = CreateObject("Penger");

    Color red = {240, 10, 10, 255};

    obj1.mesh = CreateMesh(verts, vertexCount, faces, faceCount, red);
    // obj1.transform.rotation.w = 0;
    // obj1.transform.rotation.y = 1;
    printf("Objects name is: %s\n", obj1.name);

    AddGlobalObject(obj1);
    AddObjectToScene(&testScene, &obj1);

    // Creating Second Object
    // -----------------------------------------------------------------------
    vertexCount = sizeof(Cverts) / sizeof(Cverts[0]);
    faceCount = sizeof(Cfaces) / sizeof(Cfaces[0]);

    Object obj2 = CreateObject("Sword");

    Color green = {10, 245, 10, 255};

    // obj2.mesh = CreateMesh(Cverts, vertexCount, Cfaces, faceCount, green);
    obj2.mesh = load_obj_mesh("E:/Programs/PrismCore Engine/src/SampleObjects/Sword-lowpoly.obj", green);
    // obj2.mesh = load_obj_mesh("E:/Programs/SDRenderer/src/Human.obj", green);
    obj2.transform.position.x = 1.5f;
    // obj2.transform.rotation = (Quaternion){0, 0, 0, 1};
    obj2.transform.scale = (Vector3){0.02f, 0.02f, 0.02f};
    printf("Objects2 name is: %s\n", obj2.name);

    AddGlobalObject(obj2);
    AddObjectToScene(&testScene, &obj2);
    // -----------------------------------------------------------------------
    
    // Creating camera
    // -----------------------------------------------------------------------
    Camera cam;
    cam.transform.position = (Vector3){0, 0, 0};
    cam.rotation = (Quaternion){0, 0, 0, 1};
    testScene.mainCam = &cam;
    // -----------------------------------------------------------------------


    // vertexCount = sizeof(verts) / sizeof(verts[0]);
    // faceCount = sizeof(faces) / sizeof(faces[0]);





    // Delta time constant
    // const float dt = 1.0/program.FPS;

    // Light direction
    Vector3 lightDirWorld = Vector3Normalize((Vector3){0.5f, -1.0f, 0.5f});
    

    // Variables for animation
    float dz = 1;
    float angle = 0;

    // Values for mouse movement
    float dx, dy;

    // Values for camera speed
    float speed;
    int forward, right, up;

    // Values for keyboard input:
    bool key_w = false;
    bool key_s = false;
    bool key_a = false;
    bool key_d = false;
    bool key_space = false;
    bool key_ctrl = false;


    // Main Loop
    printf("Starting main loop\n");
    int quit = 0;
    bool mouseGrabbed = false;
    bool Wireframe = false;
    bool renderDebugRays = false;
    SDL_Event event;

    // Variables for delta time
    uint64_t currentTime = SDL_GetPerformanceCounter();
    uint64_t lastTime = 0;
    double dt = 0;

    while (quit == 0)
    {
        lastTime = currentTime;
        currentTime = SDL_GetPerformanceCounter();
        dt = (double)((currentTime - lastTime) / (double)SDL_GetPerformanceFrequency());

        while (SDL_PollEvent(&event))
        {
            // If the window gets closed
            if (event.type == SDL_EVENT_QUIT)
                quit = 1;
            
            // If the window gets resized, update the program struct
            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                SDL_GetWindowSizeInPixels(window, &program.width, &program.height);

                // For Software Renderer
                if (SDL_GetRendererName(renderer)[0] == 's')
                {
                    windowSurface = SDL_GetWindowSurface(window);
                    SDL_DestroySurface(surface);
                    surface = SDL_CreateSurface(program.width, program.height, SDL_PIXELFORMAT_RGBA8888);
                    SDL_DestroyRenderer(renderer);
                    renderer = SDL_CreateSoftwareRenderer(surface);
                }
            }

            // If the button gets pressed (specifically the escape key)
            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.scancode == SDL_SCANCODE_ESCAPE)
                {
                    mouseGrabbed = false;
                    SDL_SetWindowRelativeMouseMode(window, mouseGrabbed);
                }
                if (event.key.scancode == SDL_SCANCODE_P)
                {
                    Wireframe = !Wireframe;
                }
                if (event.key.scancode == SDL_SCANCODE_L)
                {
                    renderDebugRays = !renderDebugRays;
                    printf("Render Debug Rays: ");
                    if (renderDebugRays == true) printf("On\n");
                    else                         printf("Off\n");

                    // if (renderDebugRays == true)
                    //     InitDebugLine();
                }
            }


            // If the mouse get clicked 
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                //
                if (event.button.button == SDL_BUTTON_LEFT && mouseGrabbed == true)
                {
                    Ray ray = CreateRay(&cam);
                    // ray.direction = Vector3Scale(ray.direction, -1.0f);
                    float dist;
                    Object* hitObj = RaycastScene(ray, GlobalObjects, GlobalObjectCount, &dist);

                    if (hitObj)
                        printf("Hit object: %s at distance: %f\n", hitObj->name, dist);
                    
                    AddRay(ray);
                }

                // Capture the mouse if it isn't already
                if (!SDL_GetWindowRelativeMouseMode(window))
                {
                    SDL_SetWindowRelativeMouseMode(window, true);
                    mouseGrabbed = true;
                }
            }

            // If the mouse moves and the mouse is grabbed, move the camera
            if (event.type == SDL_EVENT_MOUSE_MOTION)
            {
                if (mouseGrabbed == true)
                {
                    SDL_GetRelativeMouseState(&dx, &dy);
                    Camera_MouseLook(&cam, dx, dy, 0.002f);
                }
            }
        }

        // Get keyboard state and determine input
        const bool* state = SDL_GetKeyboardState(NULL);
        forward = state[SDL_SCANCODE_W] - state[SDL_SCANCODE_S];
        right = state[SDL_SCANCODE_D] - state[SDL_SCANCODE_A];
        // up = state[SDL_SCANCODE_SPACE] - state[SDL_SCANCODE_LCTRL];
        up = state[SDL_SCANCODE_SPACE];

        // Change speed if left shift or left control are held
        if (state[SDL_SCANCODE_LSHIFT] == true)
            speed = 6.0f * dt;
        else if (state[SDL_SCANCODE_LCTRL] == true)
            speed = 0.5f * dt;
        else
            speed = 3.0f * dt;

        // Move camera
        Camera_Move(&cam, forward, right, up, speed);

        // Setting animation variables
        // angle = 3.14159265 / 128;
        angle = 100.0f * (3.14159265f / 180.0f) * dt;



        /////////////////////////
        /// Rendering section ///
        /////////////////////////

        // Set Background
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderClear(renderer);

        //  Rotate object for an animation
        // RotateObjectZ(&GlobalObjects[0], -angle);
        RotateObjectX(&testScene.objects[1], angle);
        RotateObjectY(&testScene.objects[0], -angle);


        if (renderDebugRays == true && GlobalRayCount > 0)
        {
            RenderDebugRays(renderer, program, testScene.mainCam, GlobalRays, GlobalRayCount);
        }
        
        // Render all objects
        RenderScene(renderer, program, &testScene, lightDirWorld, Wireframe);
        


        // Needed to keep image
        SDL_RenderPresent(renderer);

        // Needed if using a software renderer
        
        if (SDL_GetRendererName(renderer)[0] == 's')
        {
            SDL_BlitSurface(surface, NULL, windowSurface, NULL);
            SDL_UpdateWindowSurface(window);
        }

        SDL_Delay(1000/program.FPS);
    }


    // Exiting functions
    printf("Quitting SDL\n");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();


    return 0;
}