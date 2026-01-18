#ifndef HARDWARE_RENDER_H
#define HARDWARE_RENDER_H

#include "structures.h"
#include "SDL3/SDL.h"



//////////////////////
// Rendering functions
//////////////////////

unsigned int CreateShaderProgram();

void InitDebugLine();
void UpdateDebugRay(Ray* rays, int rayCount);
void DrawDebugRay(unsigned int shaderProgram, Matrix4 view, Matrix4 projection, int rayCount);



void UploadMeshToGPU(Mesh* mesh);
void CalculateNormals(Vector3* vertices, int vCount, int* indices, int iCount, Vector3* outNormals);
void RenderSceneGL(SDL_Window* window, Scene* scene, unsigned int shaderProgram, Vector3 WorldLight, int renderMode, bool debugRays, int debugRayCount);


#endif