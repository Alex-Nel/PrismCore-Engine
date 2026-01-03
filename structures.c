#include <stdlib.h>
#include <string.h>
#include "structures.h"

Object CreateObject(char* name)
{
    Transform objTransform = {0.0, 0.0, 1.0};

    Object obj;
    obj.transform = objTransform;
    obj.name = name;
    obj.mesh = NULL;

    return obj;
}

Mesh* CreateMesh(Point* verts, int vertexCount, int (*faces)[3], int faceCount)
{
    // Get vertex and face counts (Will have to make a solution to this)
    // int vertexCount = sizeof(verts) / sizeof(verts[0]);
    // int faceCount = sizeof(faces) / sizeof(faces[0]);

    // Make the Mesh and allocate the size
    Mesh* objMesh = malloc(sizeof(Mesh) + vertexCount * sizeof(Point));
    if (!objMesh) return NULL;

    // Set count variables
    objMesh->vertexCount = vertexCount;
    objMesh->facesCount = faceCount;

    // memcpy the vertices and faces vectors
    objMesh->faces = malloc(faceCount * sizeof(*objMesh->faces));
    memcpy(objMesh->faces, faces, faceCount * sizeof(*objMesh->faces));
    memcpy(objMesh->vertices, verts, vertexCount * sizeof(Point));

    return objMesh;
}