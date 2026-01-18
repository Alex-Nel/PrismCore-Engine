#include "include/glad/glad.h"
#include "include/SDL3/SDL.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "structures.h"
#include "hardwareRender.h"
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

    // Setting openGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    printf("Set openGL attributes\n");
    
    // SDL Window and Renderer creation
    SDL_Window* window = SDL_CreateWindow("PrismCore", program.width, program.height, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    printf("SDL window Success\n");

    SDL_GLContext context = SDL_GL_CreateContext(window);
    printf("Made GLContext\n");
    SDL_GL_SetSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    printf("Loaded GLAD and enabled depth testing\n");

    unsigned int shaderProgram = CreateShaderProgram();
    printf("set up chaders\n");


    


    Color red = {240, 10, 10, 255};
    Color green = {10, 245, 10, 255};

    Scene testScene = {0};
    

    // Creating an object
    // -----------------------------------------------------------------------
    printf("Creating object\n");
    int vertexCount = sizeof(verts) / sizeof(verts[0]);
    int faceCount = sizeof(faces) / sizeof(faces[0]);

    Object obj1 = CreateObject("Penger");
    
    obj1.mesh = CreateMesh(verts, vertexCount, faces, faceCount, red);
    // obj1.mesh = UploadMeshToGPU(obj1Mesh);

    printf("Objects name is: %s\n", obj1.name);

    AddGlobalObject(obj1);
    AddObjectToScene(&testScene, &obj1);
    

    // Creating Second Object
    // -----------------------------------------------------------------------
    printf("Making second object\n");
    vertexCount = sizeof(Cverts) / sizeof(Cverts[0]);
    faceCount = sizeof(Cfaces) / sizeof(Cfaces[0]);

    Object obj2 = CreateObject("Sword");
    
    // Mesh* obj2Mesh = CreateMesh(Cverts, vertexCount, Cfaces, faceCount, green);
    // Mesh* obj2Mesh = load_obj_mesh("E:/Programs/BlinkEngine/src/Sword-lowpoly.obj", green);
    obj2.mesh = load_obj_mesh("E:/Programs/PrismCore Engine/src/SampleObjects/Sword-lowpoly.obj", green);
    // obj2.mesh = UploadMeshToGPU(obj2Mesh);

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

    testScene.objects = GlobalObjects;
    for (int i = 0; i < testScene.objectCount; i++)
    {
        printf("Object %d: %s\n", i, testScene.objects[i].name);
    }


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
    bool quit = false;
    bool mouseGrabbed = false;
    int renderMode = 0;
    bool renderDebugRays = false;
    SDL_Event event;

    // Variables for delta time
    uint64_t currentTime = SDL_GetPerformanceCounter();
    uint64_t lastTime = 0;
    double dt = 0;

    while (quit == false)
    {
        lastTime = currentTime;
        currentTime = SDL_GetPerformanceCounter();
        dt = (double)((currentTime - lastTime) / (double)SDL_GetPerformanceFrequency());

        // printf("Polling events\n");
        while (SDL_PollEvent(&event))
        {
            // If the window gets closed
            if (event.type == SDL_EVENT_QUIT)
                quit = true;
            
            // If the window gets resized, update the program struct
            if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
                SDL_GetWindowSizeInPixels(window, &program.width, &program.height);
                glViewport(0, 0, program.width, program.height);
                printf("Window resized. Dimensions:\n");
                printf("Width: %d\n", program.width);
                printf("Height: %d\n", program.height);
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
                    renderMode = (renderMode + 1) % 3;
                    printf("Render mode is now: ");
                    if (renderMode == 0)      printf("Mesh\n");
                    else if (renderMode == 1) printf("Wireframe\n");
                    else                      printf("Dots\n");
                }
                if (event.key.scancode == SDL_SCANCODE_L)
                {
                    renderDebugRays = !renderDebugRays;
                    printf("Render Debug Rays: ");
                    if (renderDebugRays == true) printf("On\n");
                    else                         printf("Off\n");

                    if (renderDebugRays == true)
                        InitDebugLine();
                }
            }


            // If the mouse get clicked 
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
                //
                if (event.button.button == SDL_BUTTON_LEFT && mouseGrabbed == true)
                {
                    Ray ray = CreateRay(&cam);
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

        if (mouseGrabbed == true)
            SDL_WarpMouseInWindow(window, program.width/2, program.height/2);


        // printf("Getting keyboard input\n");
        // Get keyboard state and determine input
        const bool* state = SDL_GetKeyboardState(NULL);
        forward = state[SDL_SCANCODE_W] - state[SDL_SCANCODE_S];
        right = state[SDL_SCANCODE_D] - state[SDL_SCANCODE_A];
        // up = state[SDL_SCANCODE_SPACE] - state[SDL_SCANCODE_LCTRL];
        up = state[SDL_SCANCODE_SPACE];

        // Change speed if left shift or left control are held
        if (state[SDL_SCANCODE_LSHIFT] == true)
            speed = 6.0f * (float)dt;
        else if (state[SDL_SCANCODE_LCTRL] == true)
            speed = 0.5f * (float)dt;
        else
            speed = 3.0f * (float)dt;

        // Move camera
        Camera_Move(&cam, forward, right, up, speed);

        // Setting animation variables
        angle = 100.0f * (3.14159265f / 180.0f) * (float)dt;



        /////////////////////////
        /// Rendering section ///
        /////////////////////////

        //  Rotate object for an animation
        // RotateObjectZ(&GlobalObjects[1], -angle);
        RotateObjectX(&testScene.objects[1], angle);
        RotateObjectY(&testScene.objects[0], -angle);

        // // Render all objects
        if (renderDebugRays == true && GlobalRayCount > 0)
        {
            UpdateDebugRay(GlobalRays, GlobalRayCount);
        }
        RenderSceneGL(window, &testScene, shaderProgram, lightDirWorld, renderMode, renderDebugRays, GlobalRayCount);

        SDL_Delay(1000/program.FPS);
    }


    // Exiting functions
    printf("Quitting SDL\n");
    SDL_DestroyWindow(window);

    SDL_Quit();


    return 0;
}