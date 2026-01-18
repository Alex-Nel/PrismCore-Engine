#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "structures.h"
#include "softwareRender.h"
#include "SDL3/SDL.h"


// Global variables
RenderTriangle* triangleBuffer;
int triCount = 0;


//////////////////////
// Rendering functions
//////////////////////


// Renders a screenpoint at the position specified
void RenderPoint(SDL_Renderer* renderer, ScreenPoint p)
{
    if (p.x == NAN && p.y == NAN)
        return;

    const float s = 5.0;
    SDL_SetRenderDrawColor(renderer, 255, 0, 80, 255);
    SDL_FRect rect = {p.x - s/2, p.y - s/2, s, s};
    SDL_RenderFillRect(renderer, &rect);
}



// Renders a line between two points
void DrawLine(SDL_Renderer* renderer, ScreenPoint p1, ScreenPoint p2, WindowInfo wi, Color color)
{
    // skip if either point is invalid
    if (p1.x != p1.x || p1.y != p1.y || p2.x != p2.x || p2.y != p2.y)
        return;
        
    if ((p1.x < 0 && p2.x < 0) || (p1.x >= wi.width && p2.x >= wi.width) ||
        (p1.y < 0 && p2.y < 0) || (p1.y >= wi.height && p2.y >= wi.height))
        return;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderLine(renderer, p1.x, p1.y, p2.x, p2.y);
}



// Convert 2D vector to a point on a screen
// Changes a vector to a pixel position
ScreenPoint Screen(Vector2 p, WindowInfo wi)
{
    if (isnan(p.x) || isnan(p.y))
        return (ScreenPoint){NAN, NAN};

    // Find aspect ratio
    float aspectRatio;
    if (wi.width < wi.height)
        aspectRatio = wi.width / 2.0f;
    else
        aspectRatio = wi.height / 2.0f;
    
    /* Old Formula for converting points*/
    // ScreenPoint sp = {
    //     (((p.x + 1)/2)*wi.width),
    //     ((1-((p.y + 1)/2))*wi.height)
    // };

    /* New Formula for converting points (Accounts for aspect ratio) */
    ScreenPoint sp = {
        (p.x * aspectRatio + wi.width/2.0f),
        (-p.y * aspectRatio + wi.height/2.0f)  // negative because y axis flips in SDL
    };
    return sp;
}



// Converts a 3D point to a 2d Vector on a screen
// x' = x / z
// y' = y / z
// Within the range [-1 to 1]
Vector2 Project(Vector3 p)
{
    if (p.z <= 0.01f)
        return (Vector2){NAN, NAN};
    
    Vector2 v = {
        p.x / p.z,
        p.y / p.z
    };

    return v;
}



///
/// Old projection function that transform objects as well
///
Vector2 TransformAndProject(Camera* cam, Transform* obj, Vector3 p)
{

    Vector3 rotated = RotateVectorByQuaternion(p, obj->rotation);

    // World position
    Vector3 world = {
        obj->position.x + rotated.x,
        obj->position.y + rotated.y,
        obj->position.z + rotated.z
    };

    // Camera-relative position (view space)
    Vector3 rel = {
        world.x - cam->transform.position.x,
        world.y - cam->transform.position.y,
        world.z - cam->transform.position.z
    };

    Quaternion inv = {
        -cam->rotation.x,
        -cam->rotation.y,
        -cam->rotation.z,
        cam->rotation.w 
    };
    
    Vector3 view = RotateVectorByQuaternion(rel, inv);

    if (view.z <= 0.01f)
        return (Vector2){NAN, NAN};

    // float nearPlane = 0.01f;
    // if (view.z < nearPlane) view.z = nearPlane;

    Vector2 v = {
        view.x / view.z,
        view.y / view.z
    };
    // Vector2 v = {p.x / p.z, p.y / p.z};
    return v;
}



//
// Renders an objects mesh to an SDL renderer as a wireframe
//
void RenderWireframe(SDL_Renderer* renderer, WindowInfo program, Camera* cam, Object* obj)
{
    if (obj->mesh == NULL)
        return;
    
    for (int i = 0; i < obj->mesh->facesCount; ++i)
    {
        int* row = obj->mesh->faces[i];

        // int length = sizeof(row) / sizeof(row[0]);
        for (int j = 0; j < 3; ++j)
        {
            int a = row[j];
            int b = row[(j+1)%3];

            Vector3 rotatedV = {0};
            Vector3 worldV = {0};

            // First scale, then Rotate vertex A
            worldV = obj->mesh->vertices[a];
            worldV.x *= obj->transform.scale.x;
            worldV.y *= obj->transform.scale.y;
            worldV.z *= obj->transform.scale.z;
            rotatedV = RotateVectorByQuaternion(worldV, obj->transform.rotation);
            worldV.x = obj->transform.position.x + rotatedV.x;
            worldV.y = obj->transform.position.y + rotatedV.y;
            worldV.z = obj->transform.position.z + rotatedV.z;

            // Put vertex A in camera space
            Vector3 camA = CameraSpace(cam, worldV);

            // First scale, then Rotate vertex B
            worldV = obj->mesh->vertices[b];
            worldV.x *= obj->transform.scale.x;
            worldV.y *= obj->transform.scale.y;
            worldV.z *= obj->transform.scale.z;
            rotatedV = RotateVectorByQuaternion(worldV, obj->transform.rotation);
            worldV.x = obj->transform.position.x + rotatedV.x;
            worldV.y = obj->transform.position.y + rotatedV.y;
            worldV.z = obj->transform.position.z + rotatedV.z;

            // Put vertex B in camera space
            Vector3 camB = CameraSpace(cam, worldV);

            // If the vertex is behind the camera, skip it
            if (camA.z <= 0.01f || camB.z <= 0.01f)
                continue;

            // Draw a line between points A and B
            DrawLine(renderer,
                Screen(Project(camA), program),
                Screen(Project(camB), program),
                program,
                obj->mesh->color
            );
        }
    }
}





///
/// Unused Function
///
int ClipTriangleAgainstNearPlane(Vector3 inV[3], Vector3 outTris[2][3]) 
{
    const float nearPlane = 0.001f; // Must be slightly > 0 to avoid Divide-By-Zero

    Vector3* insidePoints[3]; int insideCount = 0;
    Vector3* outsidePoints[3]; int outsideCount = 0;

    // Classify vertices as inside (in front of Z) or outside (behind Z)
    for (int i = 0; i < 3; i++) {
        if (inV[i].z >= nearPlane) {
            insidePoints[insideCount++] = &inV[i];
        } else {
            outsidePoints[outsideCount++] = &inV[i];
        }
    }

    if (insideCount == 0) return 0; // All behind -> Draw nothing
    if (insideCount == 3) {
        // All in front -> Keep original
        outTris[0][0] = *insidePoints[0];
        outTris[0][1] = *insidePoints[1];
        outTris[0][2] = *insidePoints[2];
        return 1;
    }

    // Calculate intersection ratio t = (near - p1.z) / (p2.z - p1.z)
    // We strictly assume p1 is outside and p2 is inside or vice versa for the math.
    
    if (insideCount == 1 && outsideCount == 2) {
        // Triangle is mostly outside, but the tip is inside.
        // Clip to form 1 smaller triangle.
        
        // Keep the inside point
        outTris[0][0] = *insidePoints[0];

        // Clip the two edges leading to the outside points
        float t1 = (nearPlane - insidePoints[0]->z) / (outsidePoints[0]->z - insidePoints[0]->z);
        float t2 = (nearPlane - insidePoints[0]->z) / (outsidePoints[1]->z - insidePoints[0]->z);

        outTris[0][1] = Vector3Lerp(*insidePoints[0], *outsidePoints[0], t1);
        outTris[0][2] = Vector3Lerp(*insidePoints[0], *outsidePoints[1], t2);
        
        return 1;
    }

    if (insideCount == 2 && outsideCount == 1) {
        // Triangle is mostly inside, but one corner is behind.
        // This forms a Quad. Split it into 2 Triangles.
        
        // First, clip the two edges leading to the outside point
        float t1 = (nearPlane - insidePoints[0]->z) / (outsidePoints[0]->z - insidePoints[0]->z);
        float t2 = (nearPlane - insidePoints[1]->z) / (outsidePoints[0]->z - insidePoints[1]->z);

        Vector3 clipped1 = Vector3Lerp(*insidePoints[0], *outsidePoints[0], t1);
        Vector3 clipped2 = Vector3Lerp(*insidePoints[1], *outsidePoints[0], t2);

        // Triangle 1
        outTris[0][0] = *insidePoints[0];
        outTris[0][1] = *insidePoints[1];
        outTris[0][2] = clipped1;

        // Triangle 2
        outTris[1][0] = *insidePoints[1];
        outTris[1][1] = clipped2;
        outTris[1][2] = clipped1;

        return 2;
    }

    return 0;
}







///
/// Function compares triangles for depth testing
/// 
int CompareTris(const void* a, const void* b)
{
    const RenderTriangle* t1 = a;
    const RenderTriangle* t2 = b;

    if (t1->depth < t2->depth) return 1;
    if (t1->depth > t2->depth) return -1;

    return 0;
}






//////////////////////////////////////////////////////////////////
/// Fills the triangleBuffer array with faces from all objects ///
//////////////////////////////////////////////////////////////////

void AddRenderTriangles(Object* GlobalObjects, int numObjects, Camera* cam, Vector3 lightDirCamera)
{
    const float nearPlane = 0.01f;
    int facesCount = 0;
    
    // Flip lighting for consistency
    lightDirCamera = RotateVectorByQuaternion(lightDirCamera, QuaternionInverse(cam->rotation));
    lightDirCamera.z *= -1.0f;

    for (int a = 0; a < numObjects; ++a)
    {
        if (GlobalObjects[a].mesh != NULL)
            facesCount += GlobalObjects[a].mesh->facesCount;
    }

    // Use facesCount * 2 if more faces are needed
    triangleBuffer = malloc(sizeof(RenderTriangle) * facesCount);
    // triangleBuffer = malloc(sizeof(RenderTriangle) * facesCount * 2);

    if (!triangleBuffer)
    {
        printf("Failed to allocate triangle buffer\n");
        return;
    }

    // Generate triangles for every object
    for (int a = 0; a < numObjects; ++a)
    {
        Object obj = GlobalObjects[a];

        if (obj.mesh == NULL)
            continue;

        for (int i = 0; i < obj.mesh->facesCount; ++i)
        {
            int* row = obj.mesh->faces[i];

            Vector3 camVerts[3];

            // Rotate Mesh to follow the objects rotation
            for (int k = 0; k < 3; ++k)
            {
                Vector3 localVert = obj.mesh->vertices[row[k]];
                localVert.x *= -1.0f;

                localVert = (Vector3){
                    localVert.x * obj.transform.scale.x,
                    localVert.y * obj.transform.scale.y,
                    localVert.z * obj.transform.scale.z
                };

                Vector3 rotatedVert = RotateVectorByQuaternion(localVert, obj.transform.rotation);

                Vector3 world = {
                    obj.transform.position.x + rotatedVert.x,
                    obj.transform.position.y + rotatedVert.y,
                    obj.transform.position.z + rotatedVert.z
                };

                if (cam == NULL)
                {
                    printf("Camera is NULL\n");
                    return;
                }

                camVerts[k] = CameraSpace(cam, world);
                // Flip Z for right-handed system
                camVerts[k].z *= -1.0f;
            }

            // If vertices are behind the camera, don't add
            if (camVerts[0].z <= 0 || camVerts[1].z <= 0 || camVerts[2].z <= 0)
                continue;



            ///////////////////////////////////////
            // --- Near-plane clipping --- //
            ///////////////////////////////////////
            int inFront[3];
            int countInFront = 0;
            for (int k = 0; k < 3; k++)
            {
                if (camVerts[k].z >= nearPlane) {
                    inFront[k] = 1;
                    countInFront++;
                } else {
                    inFront[k] = 0;
                }
            }

            if (countInFront == 0) continue; // fully behind camera, skip

            // Helper arrays for clipped triangles
            Vector3 clipped[4]; // max 4 vertices after clipping
            int clippedCount = 0;

            if (countInFront == 3)
            {
                // all vertices in front, keep original triangle
                clipped[0] = camVerts[0];
                clipped[1] = camVerts[1];
                clipped[2] = camVerts[2];
                clippedCount = 3;
            }
            else if (countInFront == 1)
            {
                // one vertex in front, generate 2 new triangles (quad split)
                int frontIdx = (inFront[0]) ? 0 : (inFront[1] ? 1 : 2);
                int backIdx1 = (frontIdx + 1) % 3;
                int backIdx2 = (frontIdx + 2) % 3;

                Vector3 vFront = camVerts[frontIdx];
                Vector3 vBack1 = camVerts[backIdx1];
                Vector3 vBack2 = camVerts[backIdx2];

                float t1 = (nearPlane - vFront.z) / (vBack1.z - vFront.z);
                float t2 = (nearPlane - vFront.z) / (vBack2.z - vFront.z);

                Vector3 vi1 = {
                    vFront.x + (vBack1.x - vFront.x) * t1,
                    vFront.y + (vBack1.y - vFront.y) * t1,
                    nearPlane
                };
                Vector3 vi2 = {
                    vFront.x + (vBack2.x - vFront.x) * t2,
                    vFront.y + (vBack2.y - vFront.y) * t2,
                    nearPlane
                };

                // Triangle 1
                clipped[0] = vFront;
                clipped[1] = vi1;
                clipped[2] = vi2;
                clippedCount = 3;
            }
            else if (countInFront == 2)
            {
                // two vertices in front, generate 1 new triangle
                int backIdx = (inFront[0] == 0) ? 0 : (inFront[1] == 0 ? 1 : 2);
                int frontIdx1 = (backIdx + 1) % 3;
                int frontIdx2 = (backIdx + 2) % 3;

                Vector3 vBack = camVerts[backIdx];
                Vector3 vFront1 = camVerts[frontIdx1];
                Vector3 vFront2 = camVerts[frontIdx2];

                float t1 = (nearPlane - vFront1.z) / (vBack.z - vFront1.z);
                float t2 = (nearPlane - vFront2.z) / (vBack.z - vFront2.z);

                Vector3 vi1 = {
                    vFront1.x + (vBack.x - vFront1.x) * t1,
                    vFront1.y + (vBack.y - vFront1.y) * t1,
                    nearPlane
                };
                Vector3 vi2 = {
                    vFront2.x + (vBack.x - vFront2.x) * t2,
                    vFront2.y + (vBack.y - vFront2.y) * t2,
                    nearPlane
                };

                clipped[0] = vFront1;
                clipped[1] = vFront2;
                clipped[2] = vi2;
                clippedCount = 3;
            }
            // --- End near-plane clipping --- //

            // Filling in the triangle buffer with the clipped array data
            for (int t = 0; t < clippedCount; t += 3)
            {
                Vector3 v0 = clipped[t + 0];
                Vector3 v1 = clipped[t + 1];
                Vector3 v2 = clipped[t + 2];

                // Recalculate normal for the new clipped fragment
                Vector3 ab = Vector3Subtract(v1, v0);
                Vector3 ac = Vector3Subtract(v2, v0);
                Vector3 normal = Vector3Normalize(Vector3Cross(ab, ac));

                // Recalculate brightness
                float brightness = Vector3Dot(normal, Vector3Scale(lightDirCamera, -1.0f));
                if (brightness < 0) brightness = 0;
                brightness = 0.3f + 0.9f * brightness;
                if (brightness > 1) brightness = 1;

                float depth = (v0.z + v1.z + v2.z) / 3.0f;

                triangleBuffer[triCount++] = (RenderTriangle){
                    { v0, v1, v2 }, // <--- Use v0, v1, v2 here!
                    depth,
                    ColorScale(obj.mesh->color, brightness)
                };
            }
            



            // ----------- Old routine ----------- //
            // -- Doesn't use the clipped array -- //

            // Vector3 ab = Vector3Subtract(camVerts[1], camVerts[0]);
            // Vector3 ac = Vector3Subtract(camVerts[2], camVerts[0]);
            // Vector3 normal = Vector3Normalize(Vector3Cross(ab, ac));

            // Back-face culling (Don't use right now, undefined behavior)
            // if (normal.z >= 0) continue;
            // if (Vector3Dot(normal, camVerts[0]) >= 0) continue;

            // Compute brightness (directional light)
            // float brightness = Vector3Dot(normal, Vector3Scale(lightDirCamera, -1.0f));
            // if (brightness < 0) brightness = 0;
            // brightness = 0.3f + 0.9f * brightness;
            // if (brightness > 1) brightness = 1;


            
            // float depth = (camVerts[0].z + camVerts[1].z + camVerts[2].z) / 3.0f;

            // triangleBuffer[triCount++] = (RenderTriangle){
            //     { camVerts[0], camVerts[1], camVerts[2] },
            //     depth,
            //     ColorScale(obj.mesh->color, brightness)
            // };

            // ---------- Old routine ---------- //

        }
    }

    // Sort the vertices according to depth
    qsort(triangleBuffer, triCount, sizeof(RenderTriangle), CompareTris);
}





/////////////////////////////////////////////////
/// Renders all faces in triangleBuffer array ///
/////////////////////////////////////////////////

void RenderTriangles(SDL_Renderer* renderer, WindowInfo program)
{
    for (int i = 0; i < triCount; ++i)
    {
        RenderTriangle* t = &triangleBuffer[i];

        SDL_Vertex triangle[3];

        for (int k = 0; k < 3; ++k)
        {
            Vector2 projected = {
                t->v[k].x / t->v[k].z,
                t->v[k].y / t->v[k].z
            };

            ScreenPoint sp = Screen(projected, program);

            triangle[k].position.x = sp.x;
            triangle[k].position.y = sp.y;
            triangle[k].color.a = (float)t->color.a / (float)255;
            triangle[k].color.b = (float)t->color.b / (float)255;
            triangle[k].color.g = (float)t->color.g / (float)255;
            triangle[k].color.r = (float)t->color.r / (float)255;
            triangle[k].tex_coord.x = 0;
            triangle[k].tex_coord.y = 0;
        }

        SDL_RenderGeometry(renderer, NULL, triangle, 3, NULL, 0);
    }

    // Reset triangleBuffer and triCount to fill it again next frame
    free(triangleBuffer);
    triCount = 0;
}





////////////////////////////////////////////////////////////////////////////
/// This function encapsulates all rendering steps according to settings ///
////////////////////////////////////////////////////////////////////////////

void RenderScene(SDL_Renderer* renderer, WindowInfo program, Scene* scene, Vector3 lightDirCamera, bool Wireframe)
{
    Camera* cam = scene->mainCam;
    // Depending on whether Wireframe is true or not, different rendering functions are used.
    if (Wireframe == true)
    {
        for (int i = 0; i < scene->objectCount; ++i)
            RenderWireframe(renderer, program, cam, &scene->objects[i]);
    }
    else
    {
        AddRenderTriangles(scene->objects, scene->objectCount, cam, lightDirCamera);
        RenderTriangles(renderer, program);
    }
}







/////////////////////////////////////////////////////////////////////
// Old Rotation functions - These rotate the mesh, not the object ///
/////////////////////////////////////////////////////////////////////

void rotate_xz(Mesh* obj, Vector3* p, float angle)
{
    const float c = cosf(angle);
    const float s = sinf(angle);
    float originalX = p->x;
    float originalY = p->y;
    float originalZ = p->z;
    // p->x = obj->x + (((originalX - obj->x) * c) - ((originalZ - obj->z) * s));
    // p->z = obj->z + (((originalX - obj->x) * s) + ((originalZ - obj->z) * c));
    p->x = (((originalX) * c) - ((originalZ) * s));
    p->z = (((originalX) * s) + ((originalZ) * c));
}

void rotate_xy(Mesh* obj, Vector3* p, float angle)
{
    const float c = cosf(angle);
    const float s = sinf(angle);
    float originalX = p->x;
    float originalY = p->y;
    float originalZ = p->z;
    // p->x = obj->x + (((originalX - obj->x) * c) - ((originalZ - obj->z) * s));
    // p->z = obj->z + (((originalX - obj->x) * s) + ((originalZ - obj->z) * c));
    p->x = (((originalX) * c) - ((originalY) * s));
    p->y = (((originalX) * s) + ((originalY) * c));
}

void rotate_yz(Mesh* obj, Vector3* p, float angle)
{
    const float c = cosf(angle);
    const float s = sinf(angle);
    float originalX = p->x;
    float originalY = p->y;
    float originalZ = p->z;
    // p->x = obj->x + (((originalX - obj->x) * c) - ((originalZ - obj->z) * s));
    // p->z = obj->z + (((originalX - obj->x) * s) + ((originalZ - obj->z) * c));
    p->y = (((originalY) * c) - ((originalZ) * s));
    p->z = (((originalY) * s) + ((originalZ) * c));
}



////////////////////////////
// Translation functions /// (Currently unused)
////////////////////////////


void translateObjectX(Object* obj, float x)
{
    obj->transform.position.x += x;
}

void translateObjectY(Object* obj, float y)
{
    obj->transform.position.y += y;
}

void translateObjectZ(Object* obj, float z)
{
    obj->transform.position.z += z;
}



void MoveCameraForward(Camera* cam, float amount)
{
    cam->transform.position.x += amount;
}