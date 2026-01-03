#ifndef STRUCTURES_H
#define STRUCTURES_H

// 
// Structs related to displaying on a 2D screen
//
typedef struct WindowInfo
{
    int width;
    int height;
    int FPS;
} WindowInfo;

typedef struct ScreenPoint
{
    float x;
    float y;
} ScreenPoint;

typedef struct Vector2
{
    float x;
    float y;
} Vector2;




//
// Structs related to 3D objects
//
typedef struct Point
{
    float x;
    float y;
    float z;
} Point;

typedef struct Transform
{
    float x;
    float y;
    float z;
} Transform;

typedef struct Mesh
{
    int vertexCount;
    int facesCount;
    int (*faces)[3];
    Point vertices[];
} Mesh;

typedef struct Object
{
    Transform transform;
    char* name;
    Mesh* mesh;
} Object;






Object CreateObject(char* name);

Mesh* CreateMesh(Point* verts, int vertexCount, int (*faces)[3], int faceCount);


#endif