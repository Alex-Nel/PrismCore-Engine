#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "structures.h"

#ifdef __cplusplus
extern "C" {
#endif




/////////////////////////////////////////
/// Object and Mesh creation function ///
/////////////////////////////////////////

Object CreateObject(char* name)
{
    Transform objTransform = {0};
    objTransform.position.x = 0;
    objTransform.position.y = 0;
    objTransform.position.z = -1.5f;

    objTransform.rotation.x = 0;
    objTransform.rotation.y = 0;
    objTransform.rotation.z = 0;
    objTransform.rotation.w = 1;

    objTransform.scale.x = 1;
    objTransform.scale.y = 1;
    objTransform.scale.z = 1;

    Object obj;
    obj.transform = objTransform;
    obj.name = name;
    obj.mesh = NULL;

    return obj;
}

Mesh* CreateMesh(Vector3* verts, int vertexCount, int (*faces)[3], int faceCount, Color color)
{
    // Get vertex and face counts (Will have to make a solution to this)
    // int vertexCount = sizeof(verts) / sizeof(Vector3);
    // int faceCount = sizeof(faces) / (sizeof(int) * 3);

    // Make the Mesh and allocate the size
    printf("Allocating mesh size\n");
    Mesh* objMesh = (Mesh*)malloc(sizeof(Mesh) + vertexCount * sizeof(Vector3));
    if (!objMesh) return NULL;

    printf("Setting mesh attributes\n");
    // Set count variables
    objMesh->vertexCount = vertexCount;
    objMesh->facesCount = faceCount;
    objMesh->color = color;
    objMesh->gpuMesh = NULL;

    printf("Copying vertex and face data\n");
    // memcpy the vertices and faces vectors
    objMesh->faces = (int(*)[3])malloc(faceCount * sizeof(*objMesh->faces));
    memcpy(objMesh->faces, faces, faceCount * sizeof(*objMesh->faces));
    memcpy(objMesh->vertices, verts, vertexCount * sizeof(Vector3));

    return objMesh;
}










///////////////////////////////
// Functions for Quaternions //
///////////////////////////////

// Normalize a quaternion to it's basic form
Quaternion QuaternionNormalize(Quaternion q)
{
    float len = sqrtf(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
    
    if (len == 0)
        return (Quaternion){0, 0, 0, 1};
    
    float inv = 1.0f / len;

    Quaternion qr = {q.x*inv, q.y*inv, q.z*inv, q.w*inv};

    return qr;
}



// Get the quaternion representation of a specific set of axis angles
Quaternion QuaternionFromAxisAngle(float ax, float ay, float az, float angle)
{
    float half = angle * 0.5f;
    float s = sinf(half);

    Quaternion q = {ax *s, ay * s, az *s, cosf(half)};

    return QuaternionNormalize(q);
}



// Multiplying two quaternions
Quaternion QuaternionMultiply(Quaternion a, Quaternion b)
{
    Quaternion q = {
        a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w,
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z
    };

    return q;
}



// Rotating an exact vector by a quaternion
Vector3 RotateVectorByQuaternion(Vector3 v, Quaternion q)
{
    // v' = q * v * q^-1
    Quaternion vq = {v.x, v.y, v.z, 0};
    Quaternion qi = {-q.x, -q.y, -q.z, q.w};

    Quaternion r = QuaternionMultiply(QuaternionMultiply(q, vq), qi);
    Vector3 rv = {r.x, r.y, r.z};
    
    return rv;
}



//
// Get the euler pitch from a given quaternion
//
float GetPitchFromQuaternion(Quaternion q)
{
    // Standard formula: pitch = arcsin(2*(w*y - z*x))
    float v = 2.0f * (q.w*q.y - q.z*q.x);

    if (v > 1.0f) v = 1.0f;
    if (v < -1.0f) v = -1.0f;

    return asinf(v);
}



//
// Get the euler equivalent Yaw
//
float GetYawFromQuaternion(Quaternion q)
{
    // Formula: yaw = atan2(2*(w*z + x*y), 1 - 2*(y*y + z*z))
    float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
    float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);

    return atan2f(siny_cosp, cosy_cosp);
}



//
// Get the Roll equivalent from a quaternion
//
float GetRollFromQuaternion(Quaternion q)
{
    // Formula: roll = atan2(2*(w*x + y*z), 1 - 2*(x*x + y*y))
    float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
    float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    return atan2f(sinr_cosp, cosr_cosp);
}



// Get the inverse of a quaternion
Quaternion QuaternionInverse(Quaternion q)
{
    float lenSq = q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z;

    if (lenSq == 0.0f)
    {
        Quaternion qr = {0, 0, 0, 1};
        return qr; // identity fallback
    }

    Quaternion qr = {
       -q.x / lenSq,
       -q.y / lenSq,
       -q.z / lenSq,
        q.w / lenSq
    };

    return qr;
}










///////////////////////////
// Functions for Vector2 //
///////////////////////////

//
// Adding two Vector2's
//
Vector2 Vector2Add(Vector2 a, Vector2 b)
{
    Vector2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}



//
// Subtracting two vector2's
//
Vector2 Vector2Subtract(Vector2 a, Vector2 b)
{
    Vector2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}



//
// Normalize a vector2 to it's base form
//
Vector2 Vector2Normalize(Vector2 v)
{
    float length = sqrtf(v.x * v.x + v.y * v.y);
    
    if (length == 0.0f) 
    {
        Vector2 r = {0, 0};
        return r;
    }

    Vector2 result;
    result.x = v.x / length;
    result.y = v.y / length;

    return result;
}



//
// Cross product of 2 vectors
//
float Vector2Dot(Vector2 a, Vector2 b)
{
    return a.x * b.x + a.y * b.y;
}



//
// Scaling a vector2 by a scalar
//
Vector2 Vector2Scale(Vector2 v, float s)
{
    Vector2 vo = {v.x * s, v.y * s};
    return vo;
}



//
// Linearly interpolate from a start and end given an amount of time
//
Vector2 Vector2Lerp(Vector2 start, Vector2 end, float t)
{
    Vector2 v = {
        start.x + (end.x - start.x) * t,
        start.y + (end.y - start.y) * t
    };

    return v;
}










///////////////////////////
// Functions for Vector3 //
///////////////////////////

//
// Adding two vectors.
//
Vector3 Vector3Add(Vector3 a, Vector3 b)
{
    Vector3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}



//
// Subtracing two vectors.
//
Vector3 Vector3Subtract(Vector3 a, Vector3 b)
{
    Vector3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}



//
// Normalizing a vector to it's base form.
//
Vector3 Vector3Normalize(Vector3 v)
{
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

    if (length == 0.0f) 
    {
        Vector3 v = {0, 0, 0};
        return v; // avoid division by zero
    }

    Vector3 result;
    result.x = v.x / length;
    result.y = v.y / length;
    result.z = v.z / length;

    return result;
}



//
// Cross product of two vectors.
//
Vector3 Vector3Cross(Vector3 a, Vector3 b)
{
    Vector3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;

    return result;
}



//
// Dot product of two vectors.
//
float Vector3Dot(Vector3 a, Vector3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}



//
// Scaling a vector by a scalar float.
//
Vector3 Vector3Scale(Vector3 v, float s)
{
    Vector3 vo = {v.x * s, v.y * s, v.z * s};
    return vo;
}



//
// Linearly interpolate a vector from one point to anther given a time period.
//
Vector3 Vector3Lerp(Vector3 start, Vector3 end, float t) {
    Vector3 v = {
        start.x + (end.x - start.x) * t,
        start.y + (end.y - start.y) * t,
        start.z + (end.z - start.z) * t
    };

    return v;
}



//
//
//
Vector3 TransformVertex(Vector3 v, Transform t)
{
    // Scale
    v.x *= t.scale.x;
    v.y *= t.scale.y;
    v.z *= t.scale.z;

    // Rotate
    v = RotateVectorByQuaternion(v, t.rotation);

    // Translate
    v = Vector3Add(v, t.position);

    return v;
}










///////////////////////////
// Functions for Cameras //
///////////////////////////

//
// Get the forward direction of a camera
//
Vector3 GetCameraForward(Camera* cam)
{
    // Forward in camera space is +Z or -Z depending on convention; here we use -Z
    Vector3 vz = {0, 0, -1};
    return RotateVectorByQuaternion(vz, cam->rotation);
}



//
// Get the direction that's to the right of where the camera is facing.
//
Vector3 GetCameraRight(Camera* cam)
{
    Vector3 vx = {1, 0, 0};
    return RotateVectorByQuaternion(vx, cam->rotation);
}



//
// Function to move a camera in any direction by any speed.
//
void Camera_Move(Camera* cam, int forward, int right, int up, float speed)
{
    Vector3 fwd = GetCameraForward(cam);
    Vector3 rgt = GetCameraRight(cam);
    Vector3 worldUp = {0, 1, 0};

    // Forward/backward
    cam->transform.position.x += fwd.x * speed * forward;
    cam->transform.position.y += fwd.y * speed * forward;
    cam->transform.position.z += fwd.z * speed * forward;

    // Left/right
    cam->transform.position.x += rgt.x * speed * right;
    cam->transform.position.y += rgt.y * speed * right;
    cam->transform.position.z += rgt.z * speed * right;

    // Up/down (optional, e.g., flying)
    cam->transform.position.x += worldUp.x * speed * up;
    cam->transform.position.y += worldUp.y * speed * up;
    cam->transform.position.z += worldUp.z * speed * up;
}



//
// Main function for rotating the camera.
//
void Camera_MouseLook(Camera* cam, float dx, float dy, float sensitivity)
{
    Quaternion q = cam->rotation;

    float yaw = atan2f(2.0f * (q.w * q.y + q.x * q.z),
                       1.0f - 2.0f * (q.y * q.y + q.x * q.x));
    

    float pitch = asinf(2.0f * (q.w * q.x - q.z * q.y));

    yaw   -= dx * sensitivity;  // mouse left/right
    pitch -= dy * sensitivity;  // mouse up/down

    const float pitch_limit = 1.55f;
    if (pitch > pitch_limit) pitch = pitch_limit;
    if (pitch < -pitch_limit) pitch = -pitch_limit;

    Quaternion yawQ   = QuaternionFromAxisAngle(0.0f, 1.0f, 0.0f, yaw);    // world up
    Quaternion pitchQ = QuaternionFromAxisAngle(1.0f, 0.0f, 0.0f, pitch);  // local right

    // Order matters: yaw in world space, pitch in local space
    cam->rotation = QuaternionMultiply(yawQ, pitchQ);

    cam->rotation = QuaternionNormalize(cam->rotation);
}










////////////////////////////////////
// Functions for rotating objects //
////////////////////////////////////

//
// Rotate the object in the X axis by an angle.
//
void RotateObjectX(Object* obj, float angle)
{
    Vector3 axis = {1.0f, 0.0f, 0.0f};
    Quaternion rotStep = QuaternionFromAxisAngle(axis.x, axis.y, axis.z, angle);
    
    // Apply new rotation ON TOP of existing rotation
    obj->transform.rotation = QuaternionMultiply(obj->transform.rotation, rotStep);
    
    // Normalize occasionally to keep it clean
    obj->transform.rotation = QuaternionNormalize(obj->transform.rotation);
}



//
// Rotate the object in the Y axis by an angle.
//
void RotateObjectY(Object* obj, float angle)
{
    Vector3 axis = {0.0f, 1.0f, 0.0f};
    Quaternion rotStep = QuaternionFromAxisAngle(axis.x, axis.y, axis.z, angle);
    
    obj->transform.rotation = QuaternionMultiply(obj->transform.rotation, rotStep);
    obj->transform.rotation = QuaternionNormalize(obj->transform.rotation);
}



//
// Rotate the object in the Z axis by an angle.
//
void RotateObjectZ(Object* obj, float angle)
{
    Vector3 axis = {0.0f, 0.0f, 1.0f};
    Quaternion rotStep = QuaternionFromAxisAngle(axis.x, axis.y, axis.z, angle);
    
    obj->transform.rotation = QuaternionMultiply(obj->transform.rotation, rotStep);
    obj->transform.rotation = QuaternionNormalize(obj->transform.rotation);
}










//////////////////////////////////////////
// Other Functions for object variables //
//////////////////////////////////////////


//
// This function converts a vector into camera space for projection.
//
Vector3 CameraSpace(const Camera* cam, Vector3 world)
{
    // Step 1: translate a vector so that the camera is a (0, 0, 0)
    Vector3 v = {
        world.x - cam->transform.position.x,
        world.y - cam->transform.position.y,
        world.z - cam->transform.position.z
    };

    // Step 2: rotate by inverse camera rotation so movement feels natural
    Quaternion invRot = QuaternionInverse(cam->rotation);
    v = RotateVectorByQuaternion(v, invRot);

    return v;
}



//
// Scales a color by a certain brightness value
//
Color ColorScale(Color color, float brightness)
{
    // Clamp brightness to [0, 1] just in case
    if (brightness < 0.0f) brightness = 0.0f;
    if (brightness > 1.0f) brightness = 1.0f;

    Color result;
    result.r = (uint8_t)((float)color.r * brightness);
    result.g = (uint8_t)((float)color.g * brightness);
    result.b = (uint8_t)((float)color.b * brightness);
    result.a = color.a; // alpha stays the same

    return result;
}










//////////////////////////////////////
// Function for importing OBJ files //
//////////////////////////////////////


//
// Function that loads an obj file and makes a mesh out of it
//
Mesh *load_obj_mesh(const char *filename, Color color)
{
    int vertexCount = 0;
    int triangleCount = 0;

    // Count 
    // obj_count(filename, &vertexCount, &triangleCount);
    FILE *filestart = fopen(filename, "r");
    char line[512];

    int vcount = 0;
    int tcount = 0;

    while (fgets(line, sizeof(line), filestart)) {

        if (strncmp(line, "v ", 2) == 0) {
            vcount++;
        }
        else if (strncmp(line, "f ", 2) == 0) {
            int vertsInFace = 0;
            char *ptr = strtok(line + 2, " \t\r\n");
            while (ptr) {
                vertsInFace++;
                ptr = strtok(NULL, " \t\r\n");
            }

            if (vertsInFace >= 3)
                tcount += vertsInFace - 2;  // triangulation
        }
    }

    fclose(filestart);

    vertexCount = vcount;
    triangleCount = tcount;


    // Second pass

    // Allocate mesh + vertices
    Mesh *mesh = (Mesh*)malloc(sizeof(Mesh) + sizeof(Vector3) * vertexCount);
    mesh->vertexCount = vertexCount;
    mesh->facesCount = triangleCount;
    mesh->color = color;
    mesh->gpuMesh = NULL;

    // Allocate faces
    mesh->faces = (int(*)[3])malloc(sizeof(int[3]) * triangleCount);

    FILE *file = fopen(filename, "r");
    // char line[512];

    int vIndex = 0;
    int fIndex = 0;

    while (fgets(line, sizeof(line), file)) {

        /* ---- Vertex ---- */
        if (strncmp(line, "v ", 2) == 0) {
            Vector3 v;
            if (sscanf(line + 2, "%f %f %f", &v.x, &v.y, &v.z) == 3) {
                mesh->vertices[vIndex++] = v;
            }
        }

        /* ---- Face ---- */
        else if (strncmp(line, "f ", 2) == 0) {
            int indices[64];
            int count = 0;

            char *ptr = strtok(line + 2, " \t\r\n");
            while (ptr && count < 64) {
                int idx = atoi(ptr);

                if (idx > 0)
                    indices[count++] = idx - 1;
                else if (idx < 0)
                    indices[count++] = vertexCount + idx;
                else
                    indices[count++] = 0;
                
                ptr = strtok(NULL, " \t\r\n");
            }

            // Fan triangulation
            for (int i = 1; i + 1 < count; i++) {
                mesh->faces[fIndex][0] = indices[0];
                mesh->faces[fIndex][1] = indices[i];
                mesh->faces[fIndex][2] = indices[i + 1];
                fIndex++;
            }
        }
    }

    fclose(file);
    return mesh;
}










////////////////////////////////
// Function for for Matrix4's //
////////////////////////////////


//
// Get the identity matrix.
//
Matrix4 Mat4Identity()
{
    Matrix4 m = {0};
    
    m.m0 = 1.0f; m.m5 = 1.0f; m.m10 = 1.0f; m.m15 = 1.0f;

    return m;
}



//
// Multiply two 4x4 Matrices.
//
Matrix4 Mat4Multiply(Matrix4 A, Matrix4 B)
{
    Matrix4 out;

    // Column 1
    out.m0  = A.m0*B.m0  + A.m4*B.m1  + A.m8*B.m2  + A.m12*B.m3;
    out.m1  = A.m1*B.m0  + A.m5*B.m1  + A.m9*B.m2  + A.m13*B.m3;
    out.m2  = A.m2*B.m0  + A.m6*B.m1  + A.m10*B.m2 + A.m14*B.m3;
    out.m3  = A.m3*B.m0  + A.m7*B.m1  + A.m11*B.m2 + A.m15*B.m3;

    // Column 2
    out.m4  = A.m0*B.m4  + A.m4*B.m5  + A.m8*B.m6  + A.m12*B.m7;
    out.m5  = A.m1*B.m4  + A.m5*B.m5  + A.m9*B.m6  + A.m13*B.m7;
    out.m6  = A.m2*B.m4  + A.m6*B.m5  + A.m10*B.m6 + A.m14*B.m7;
    out.m7  = A.m3*B.m4  + A.m7*B.m5  + A.m11*B.m6 + A.m15*B.m7;

    // Column 3
    out.m8  = A.m0*B.m8  + A.m4*B.m9  + A.m8*B.m10 + A.m12*B.m11;
    out.m9  = A.m1*B.m8  + A.m5*B.m9  + A.m9*B.m10 + A.m13*B.m11;
    out.m10 = A.m2*B.m8  + A.m6*B.m9  + A.m10*B.m10+ A.m14*B.m11;
    out.m11 = A.m3*B.m8  + A.m7*B.m9  + A.m11*B.m10+ A.m15*B.m11;

    // Column 4
    out.m12 = A.m0*B.m12 + A.m4*B.m13 + A.m8*B.m14 + A.m12*B.m15;
    out.m13 = A.m1*B.m12 + A.m5*B.m13 + A.m9*B.m14 + A.m13*B.m15;
    out.m14 = A.m2*B.m12 + A.m6*B.m13 + A.m10*B.m14+ A.m14*B.m15;
    out.m15 = A.m3*B.m12 + A.m7*B.m13 + A.m11*B.m14+ A.m15*B.m15;

    return out;
}



//
// Function to get the Perspective Projection Matrix.
//
Matrix4 Mat4Perspective(float fovRadians, float aspect, float nearZ, float farZ)
{
    Matrix4 m = Mat4Identity();
    // float tanHalfFov = tanf(fovRadians / 2.0f);
    float t = tanf(fovRadians * 0.5f);
    
    m.m0 = 1.0f / (aspect * t);
    m.m5 = 1.0f / (t);
    m.m10 = -(farZ + nearZ) / (farZ - nearZ);
    m.m11 = -1.0f;
    m.m14 = -(2.0f * farZ * nearZ) / (farZ - nearZ);
    m.m15 = 0.0f;
    
    return m;
}



//
// Quaternion to Rotation Matrix.
//
Matrix4 Mat4FromQuaternion(Quaternion q)
{
    Matrix4 m = Mat4Identity();
    
    float xx = q.x * q.x; float yy = q.y * q.y; float zz = q.z * q.z;
    float xy = q.x * q.y; float xz = q.x * q.z; float yz = q.y * q.z;
    float wx = q.w * q.x; float wy = q.w * q.y; float wz = q.w * q.z;

    m.m0 = 1.0f - 2.0f * (yy + zz);
    m.m1 = 2.0f * (xy + wz);
    m.m2 = 2.0f * (xz - wy);

    m.m4 = 2.0f * (xy - wz);
    m.m5 = 1.0f - 2.0f * (xx + zz);
    m.m6 = 2.0f * (yz + wx);

    m.m8 = 2.0f * (xz + wy);
    m.m9 = 2.0f * (yz - wx);
    m.m10 = 1.0f - 2.0f * (xx + yy);

    return m;
}



//
// Get the Matrix from a corresponding objects Mesh (Position + Rotation + Scale).
//
Matrix4 GetModelMatrix(Transform t)
{
    // Start with Rotation
    Matrix4 m = Mat4FromQuaternion(t.rotation);

    // Apply Scale (Multiply diagonal elements)
    m.m0 *= t.scale.x; m.m1 *= t.scale.x; m.m2 *= t.scale.x;
    m.m4 *= t.scale.y; m.m5 *= t.scale.y; m.m6 *= t.scale.y;
    m.m8 *= t.scale.z; m.m9 *= t.scale.z; m.m10 *= t.scale.z;

    // Apply Translation (Last column)
    m.m12 = t.position.x;
    m.m13 = t.position.y;
    m.m14 = t.position.z;

    return m;
}



//
// Get the View Matrix (Inverse of Camera).
//
Matrix4 GetViewMatrix(Camera* cam)
{
    // To create a view matrix, we effectively want the INVERSE of the camera's model matrix.
    // Invert Rotation
    Quaternion invRot = QuaternionInverse(cam->rotation);
    
    Matrix4 rot = Mat4FromQuaternion(invRot);

    // Invert Position (Translation needs to be applied after rotation in view space)
    // The standard View Matrix formula: Rot_Inverse * Translation_Inverse
    Matrix4 trans = Mat4Identity();
    trans.m12 = -cam->transform.position.x;
    trans.m13 = -cam->transform.position.y;
    trans.m14 = -cam->transform.position.z;

    return Mat4Multiply(rot, trans);        // Rot * Trans
}



//
// Transpose of a Matrix.
//
Matrix4 Mat4Transpose(Matrix4 m)
{
    Matrix4 r = m;

    r.m1  = m.m4;  r.m4  = m.m1;
    r.m2  = m.m8;  r.m8  = m.m2;
    r.m3  = m.m12; r.m12 = m.m3;

    r.m6  = m.m9;  r.m9  = m.m6;
    r.m7  = m.m13; r.m13 = m.m7;

    r.m11 = m.m14; r.m14 = m.m11;

    return r;
}



//
// Get the resulting Matrix when looking at a target.
//
Matrix4 Mat4LookAt(Vector3 eye, Vector3 target, Vector3 up)
{
    Vector3 f = Vector3Normalize(Vector3Subtract(target, eye));
    Vector3 s = Vector3Normalize(Vector3Cross(f, up));
    Vector3 u = Vector3Cross(s, f);

    Matrix4 m = Mat4Identity();

    m.m0 = s.x;  m.m4 = s.y;  m.m8  = s.z;
    m.m1 = u.x;  m.m5 = u.y;  m.m9  = u.z;
    m.m2 = -f.x; m.m6 = -f.y; m.m10 = -f.z;

    m.m12 = -Vector3Dot(s, eye);
    m.m13 = -Vector3Dot(u, eye);
    m.m14 =  Vector3Dot(f, eye);

    return m;
}



//
// The ortho matrix.
//
Matrix4 Mat4Ortho(float left, float right, float bottom, float top, float nearZ, float farZ)
{
    Matrix4 m = Mat4Identity();

    m.m0  = 2.0f / (right - left);
    m.m5  = 2.0f / (top - bottom);
    m.m10 = -2.0f / (farZ - nearZ);

    m.m12 = -(right + left) / (right - left);
    m.m13 = -(top + bottom) / (top - bottom);
    m.m14 = -(farZ + nearZ) / (farZ - nearZ);

    return m;
}



//
// Getting the inverse of a matrix.
//
Matrix4 Mat4Inverse(Matrix4 m)
{
    // Extract rotation (upper 3x3)
    Matrix4 r = m;

    // Transpose rotation
    r = Mat4Transpose(r);

    // Extract translation
    Vector3 t = {m.m12, m.m13, m.m14};

    // New translation = -(R^T * t)
    Vector3 nt = {
        -(r.m0 * t.x + r.m4 * t.y + r.m8  * t.z),
        -(r.m1 * t.x + r.m5 * t.y + r.m9  * t.z),
        -(r.m2 * t.x + r.m6 * t.y + r.m10 * t.z)
    };

    r.m12 = nt.x;
    r.m13 = nt.y;
    r.m14 = nt.z;

    return r;
}



//
// Translating a Matrix by a certain value.
//
Matrix4 Mat4Translate(Vector3 t)
{
    Matrix4 m = Mat4Identity();
    m.m12 = t.x;
    m.m13 = t.y;
    m.m14 = t.z;

    return m;
}



//
// Scaling a matrix by a scalar vector.
//
Matrix4 Mat4Scale(Vector3 s)
{
    Matrix4 m = Mat4Identity();
    m.m0 = s.x;
    m.m5 = s.y;
    m.m10 = s.z;

    return m;
}



//
// Rotate a matrix by an angle.
//
Matrix4 Mat4RotateX(float r)
{
    Matrix4 m = Mat4Identity();
    float c = cosf(r), s = sinf(r);
    m.m5 = c;  m.m9  = -s;
    m.m6 = s;  m.m10 = c;

    return m;
}



//
// Get the array representation of a matrix.
//
void Mat4ToArray(Matrix4 m, float out[16])
{
    memcpy(out, &m, sizeof(Matrix4));
}










///////////////////////////
// Function for for Rays //
///////////////////////////

//
// Creates a ray from a specified camera
//
Ray CreateRay(Camera* cam)
{
    Ray r;
    r.origin = cam->transform.position;
    r.direction = Vector3Normalize(GetCameraForward(cam));
    return r;
}



//
// Checks if a ray intersects with a triangle
//
bool RayIntersectsTriangle(Ray ray, Vector3 v0, Vector3 v1, Vector3 v2, float* outT)
{
    const float EPSILON = 0.000001f;

    Vector3 edge1 = Vector3Subtract(v1, v0);
    Vector3 edge2 = Vector3Subtract(v2, v0);

    Vector3 h = Vector3Cross(ray.direction, edge2);
    float a = Vector3Dot(edge1, h);

    if (a > -EPSILON && a < EPSILON)
        return false; // Ray parallel to triangle

    float f = 1.0f / a;
    Vector3 s = Vector3Subtract(ray.origin, v0);
    float u = f * Vector3Dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return false;

    Vector3 q = Vector3Cross(s, edge1);
    float v = f * Vector3Dot(ray.direction, q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    float t = f * Vector3Dot(edge2, q);

    if (t > EPSILON)
    {
        if (outT) *outT = t;
        return true;
    }

    return false;
}



//
// Function to check if a ray instersects with an objects mesh
//
bool RayIntersectsMesh(Ray ray, Object* obj, float* outDistance)
{
    Mesh* mesh = obj->mesh;
    bool hit = false;
    float closest = 1e30f;

    for (int i = 0; i < mesh->facesCount; i++)
    {
        int i0 = mesh->faces[i][0];
        int i1 = mesh->faces[i][1];
        int i2 = mesh->faces[i][2];

        Vector3 v0 = TransformVertex(mesh->vertices[i0], obj->transform);
        Vector3 v1 = TransformVertex(mesh->vertices[i1], obj->transform);
        Vector3 v2 = TransformVertex(mesh->vertices[i2], obj->transform);

        float t;
        if (RayIntersectsTriangle(ray, v0, v1, v2, &t))
        {
            if (t < closest)
            {
                closest = t;
                hit = true;
            }
        }
    }

    if (hit && outDistance)
        *outDistance = closest;

    return hit;
}



//
// Main function to check which objects a ray intersects with
//
Object* RaycastScene(Ray ray, Object* objects, int objCount, float* outDistance)
{
    Object* hitObj = NULL;
    float closest = 1e30f;

    for (int i = 0; i < objCount; i++)
    {
        float t;
        if (RayIntersectsMesh(ray, &objects[i], &t))
        {
            if (t < closest)
            {
                closest = t;
                hitObj = &objects[i];
            }
        }
    }

    if (hitObj && outDistance)
        *outDistance = closest;

    return hitObj;
}




#ifdef __cplusplus
}
#endif