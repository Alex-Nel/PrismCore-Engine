#ifndef SOFTWARE_RENDER_H
#define SOFTWARE_RENDER_H

#include "structures.h"
#include "SDL3/SDL.h"


// // Global variables
// RenderTriangle* triangleBuffer;
// int triCount = 0;



//////////////////////
// Rendering functions
//////////////////////


// Renders a screenpoint at the position specified
void RenderPoint(SDL_Renderer* renderer, ScreenPoint p);



// Renders a line between two points
void DrawLine(SDL_Renderer* renderer, ScreenPoint p1, ScreenPoint p2, WindowInfo wi, Color color);



// Convert 2D vector to a point on a screen
// Changes a vector to a pixel position
ScreenPoint Screen(Vector2 p, WindowInfo wi);



// Converts a 3D point to a 2d Vector on a screen
// x' = x / z
// y' = y / z
// Within the range [-1 to 1]
Vector2 Project(Vector3 p);


// Legacy function - Do not use
Vector2 TransformAndProject(Camera* cam, Transform* obj, Vector3 p);

// Wireframe renderer
void RenderWireframe(SDL_Renderer* renderer, WindowInfo program, Camera* cam, Object* obj);

// Functions for mesh rendering
int ClipTriangleAgainstNearPlane(Vector3 inV[3], Vector3 outTris[2][3]);
int CompareTris(const void* a, const void* b);
void AddRenderTriangles(Object* GlobalObjects, int numObjects, Camera* cam, Vector3 lightDirCamera);
void RenderTriangles(SDL_Renderer* renderer, WindowInfo program);

// Full render function to encapsulate all settings
void RenderObjects(SDL_Renderer* renderer, WindowInfo program, Object* GlobalObjects, int numObjects, Camera* cam, Vector3 lightDirCamera, bool Wireframe);





////////////////////////////////////////
// Rotation and translation functions //
////////////////////////////////////////

void rotate_xz(Mesh* obj, Vector3* p, float angle);
void rotate_xy(Mesh* obj, Vector3* p, float angle);
void rotate_yz(Mesh* obj, Vector3* p, float angle);


void translateObjectX(Object* obj, float x);
void translateObjectY(Object* obj, float y);
void translateObjectZ(Object* obj, float z);



void MoveCameraForward(Camera* cam, float amount);


#endif