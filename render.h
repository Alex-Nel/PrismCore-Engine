#include <math.h>
#include "structures.h"
#include "SDL3/SDL.h"

//////////////////////
// Rendering functions
//////////////////////


// Renders a screenpoint at the position specified
void RenderPoint(SDL_Renderer* renderer, ScreenPoint p)
{
    const float s = 5.0;
    SDL_SetRenderDrawColor(renderer, 255, 0, 80, 255);
    SDL_FRect rect = {p.x - s/2, p.y - s/2, s, s};
    SDL_RenderFillRect(renderer, &rect);
}



// Convert 2D vector to a point on a screen
// Changes a vector to a pixel position
ScreenPoint Screen(Vector2 p, WindowInfo wi)
{
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
Vector2 Project(Transform* obj, Point p)
{
    Vector2 v = {(obj->x + p.x) / (obj->z + p.z), (obj->y + p.y) / (obj->z + p.z)};
    // Vector2 v = {p.x / p.z, p.y / p.z};
    return v;
}





//////////////////////
// Rotation functions
//////////////////////


void rotate_xz(Mesh* obj, Point* p, float angle)
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

void rotate_xy(Mesh* obj, Point* p, float angle)
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

void rotate_yz(Mesh* obj, Point* p, float angle)
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


void translateObjectX(Object* obj, float x)
{
    obj->transform.x += x;
}

void translateObjectY(Object* obj, float y)
{
    obj->transform.y += y;
}

void translateObjectZ(Object* obj, float z)
{
    obj->transform.z += z;
}