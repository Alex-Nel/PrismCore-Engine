#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////
/// Structs related to displaying on a 2D screen ///
////////////////////////////////////////////////////

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




/////////////////////////////////////
/// Structs related to 3D objects ///
/////////////////////////////////////


// A RGBA representation of a color
typedef struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;


// A 3D vector
typedef struct Vector3
{
    float x;
    float y;
    float z;
} Vector3;


// Used to represent rotation
typedef struct Quaternion
{
    float x;
    float y;
    float z;
    float w;
} Quaternion;


// Struct to hold the GPU pointers for it's corresponding Mesh
typedef struct GPUMesh
{
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int NormalVBO;
} GPUMesh;


// Triangle structure used for software rendering
typedef struct RenderTriangle
{
    Vector3 v[3];
    float depth;
    Color color;
} RenderTriangle;


// The mesh of an object that you would render
typedef struct Mesh
{
    int vertexCount;
    int facesCount;
    Color color;
    GPUMesh* gpuMesh;
    int (*faces)[3];
    Vector3 vertices[];
} Mesh;


// A transform of an object
typedef struct Transform
{
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
} Transform;


// An object that encapsulates a transform, name, and mesh
typedef struct Object
{
    Transform transform;
    char* name;
    Mesh* mesh;
} Object;


// A camera used to determine rendering
typedef struct Camera
{
    Transform transform;
    Quaternion rotation;
} Camera;


// A scene contains several objects
typedef struct Scene
{
    Camera* mainCam;
    int objectCount;
    int objectCapacity;
    Object* objects;
} Scene;


// A 4x4 matrix used for openGL
typedef struct Matrix4
{
    float m0, m1, m2,  m3;
    float m4, m5, m6,  m7;
    float m8, m9, m10, m11;
    float m12, m13, m14, m15;
} Matrix4;



typedef struct Ray
{
    Vector3 origin;
    Vector3 direction;
} Ray;




/////////////////////////////////////////
/// Functions that use the structures ///
/////////////////////////////////////////


// Creating an object
Object CreateObject(char* name);


// Creating a mesh
Mesh* CreateMesh(Vector3* verts, int vertexCount, int (*faces)[3], int faceCount, Color color);


// Quaternion operations
Quaternion QuaternionNormalize(Quaternion q);
Quaternion QuaternionFromAxisAngle(float ax, float ay, float az, float angle);
Quaternion QuaternionMultiply(Quaternion a, Quaternion b);
Vector3 RotateVectorByQuaternion(Vector3 v, Quaternion q);
float GetPitchFromQuaternion(Quaternion q);
float GetYawFromQuaternion(Quaternion q);
float GetRollFromQuaternion(Quaternion q);
Quaternion QuaternionInverse(Quaternion q);



// Vector2 operations
Vector2 Vector2Add(Vector2 a, Vector2 b);
Vector2 Vector2Subtract(Vector2 a, Vector2 b);
Vector2 Vector2Normalize(Vector2 v);
float Vector2Dot(Vector2 a, Vector2 b);
Vector2 Vector2Scale(Vector2 v, float s);
Vector2 Vector2Lerp(Vector2 start, Vector2 end, float t);



// Vector3 operations
Vector3 Vector3Add(Vector3 a, Vector3 b);
Vector3 Vector3Subtract(Vector3 a, Vector3 b);
Vector3 Vector3Normalize(Vector3 v);
Vector3 Vector3Cross(Vector3 a, Vector3 b);
float Vector3Dot(Vector3 a, Vector3 b);
Vector3 Vector3Scale(Vector3 v, float s);
Vector3 Vector3Lerp(Vector3 start, Vector3 end, float t);

Vector3 TransformVertex(Vector3 v, Transform t);


// Camera operations
Vector3 GetCameraForward(Camera* cam);
Vector3 GetCameraRight(Camera* cam);
void Camera_Move(Camera* cam, int forward, int right, int up, float speed);
void Camera_MouseLook(Camera* cam, float dx, float dy, float sensitivity);


// Object rotation functions
void RotateObjectX(Object* obj, float angle);
void RotateObjectY(Object* obj, float angle);
void RotateObjectZ(Object* obj, float angle);


// vertex rendering helper functions
Vector3 CameraSpace(const Camera* cam, Vector3 world);
Color ColorScale(Color color, float brightness);


// .obj file parser
Mesh* load_obj_mesh(const char *filename, Color color);


// Scene functions
void AddObjectToScene(Scene* scene, Object* obj);


// Matrix4 operations
Matrix4 Mat4Identity();
Matrix4 Mat4Multiply(Matrix4 A, Matrix4 B);
Matrix4 Mat4Perspective(float fovRadians, float aspect, float nearZ, float farZ);
Matrix4 Mat4FromQuaternion(Quaternion q);
Matrix4 GetModelMatrix(Transform t);
Matrix4 GetViewMatrix(Camera* cam);
Matrix4 Mat4Transpose(Matrix4 m);
Matrix4 Mat4LookAt(Vector3 eye, Vector3 target, Vector3 up);
Matrix4 Mat4Ortho(float left, float right, float bottom, float top, float nearZ, float farZ);
Matrix4 Mat4Inverse(Matrix4 m);
Matrix4 Mat4Translate(Vector3 t);
Matrix4 Mat4Scale(Vector3 s);
Matrix4 Mat4RotateX(float r);
void Mat4ToArray(Matrix4 m, float out[16]);


// Ray operations
Ray CreateRay(Camera* cam);
bool RayIntersectsTriangle(Ray ray, Vector3 v0,Vector3 v1, Vector3 v2, float* outT);
bool RayIntersectsMesh(Ray ray, Object* obj, float* outDistance);
Object* RaycastScene(Ray ray, Object* objects, int objCount, float* outDistance);


#ifdef __cplusplus
}
#endif


#endif