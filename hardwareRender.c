#include "include/glad/glad.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "structures.h"
#include "hardwareRender.h"
#include "SDL3/SDL.h"


unsigned int debugLineVAO = 0;
unsigned int debugLineVBO = 0;


// 1. The Vertex Shader Source
//    It takes a generic 3D point (aPos) and multiplies it by our 3 matrices.
const char* vertexShaderSource = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"

    "out vec3 Normal;\n"     // Output to Fragment Shader
    "out vec3 FragPos;\n"    // Output to Fragment Shader (for advanced lighting later)

    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"

    "void main()\n"
    "{\n"
        "gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "FragPos = vec3(model * vec4(aPos, 1.0));\n"
        
        // Rotate the normal to match the object's rotation
        // (Casting to mat3 removes translation, which normals don't need)
        "Normal = mat3(model) * aNormal;\n" 
    "}\n";





// 2. The Fragment Shader Source
//    It outputs a single color for the shape.
const char* fragmentShaderSource = 
    "#version 330 core\n"
    "out vec4 FragColor;\n"

    "in vec3 Normal;\n"  // From Vertex Shader
    "in vec3 FragPos;\n"

    "uniform vec3 objectColor;\n"
    "uniform vec3 lightDir;\n"   // The direction of the sun
    "uniform int renderMode;\n"

    "void main()\n"
    "{\n"
        "if (renderMode == 1 || renderMode == 2) {\n"
            "FragColor = vec4(objectColor, 1.0);\n"
        "}\n"
        "else {\n"
            // 1. Ambient (Base brightness so shadows aren't pitch black)
            "float ambientStrength = 0.3;\n"
            "vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);\n"
        
            // 2. Diffuse (Directional Light)
            "vec3 norm = normalize(Normal);\n"
            // We reverse lightDir because standard math expects direction TO the light source
            "vec3 lightDirNormalized = normalize(-lightDir);\n" 
            
            "float diff = max(dot(norm, lightDirNormalized), 0.0);\n"
            "vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);\n"
            
            // 3. Combine
            "vec3 result = (ambient + diffuse) * objectColor;\n"
            "FragColor = vec4(result, 1.0);\n"
        "}\n"
    "}\n";






// 3. The Compiler Helper
//    Compiles the strings into a GPU program.
unsigned int CreateShaderProgram()
{
    // A. Compile Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check for errors (Optional but recommended)
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // B. Compile Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    // Check for errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // C. Link them into a Program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // D. Clean up (We don't need the individual objects anymore)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}










//////////////////////////
// Rendering functions ///
//////////////////////////


//
// Function initializes the debug line buffers
//
void InitDebugLine()
{
    glGenVertexArrays(1, &debugLineVAO);
    glGenBuffers(1, &debugLineVBO);

    glBindVertexArray(debugLineVAO);

    glBindBuffer(GL_ARRAY_BUFFER, debugLineVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(Vector3) * 2 * 10000,
        NULL,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}



//
// Use this to update all debug rays at once
//
void UpdateDebugRay(Ray* rays, int rayCount)
{
    if (rayCount > 10000) rayCount = 10000;
    if (rayCount == 0) return;

    Vector3* lineVerts = malloc(sizeof(Vector3) * 2 * rayCount);

    for (int i = 0; i < rayCount; i++)
    {
        lineVerts[i*2] = rays[i].origin;
        lineVerts[i*2+1] = Vector3Add(rays[i].origin, Vector3Scale(rays[i].direction, 100.0f));
    }

    glBindBuffer(GL_ARRAY_BUFFER, debugLineVBO);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vector3) * 2 * rayCount, lineVerts);

    free(lineVerts);
}



//
// Function to draw all debug rays
//
void DrawDebugRay(unsigned int shaderProgram, Matrix4 view, Matrix4 projection, int rayCount)
{
    if (rayCount > 10000) rayCount = 10000;
    if (rayCount == 0) return;
    
    glUseProgram(shaderProgram);

    glLineWidth(1.0f);

    glUniform1i(glGetUniformLocation(shaderProgram, "renderMode"), 1);
    
    Matrix4 model = Mat4Identity();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, (float*)&model);
    
    glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 1.0f, 0.0f, 0.0f);

    glBindVertexArray(debugLineVAO);
    
    // DRAW COMMAND: Draw 2 vertices for every ray
    glDrawArrays(GL_LINES, 0, rayCount * 2);
}



//
// Makes a GPUMesh for an object
//
void UploadMeshToGPU(Mesh* mesh)
{
    if (!mesh) return;

    if (mesh->gpuMesh != NULL) return;

    // GPUMesh* gpu = malloc(sizeof(GPUMesh));
    // gpu->cpuMesh = cpuMesh; // Store the reference
    mesh->gpuMesh = malloc(sizeof(GPUMesh));

    // 1. Calculate Normals (Temp buffer)
    Vector3* normals = malloc(mesh->vertexCount * sizeof(Vector3));
    CalculateNormals(mesh->vertices, mesh->vertexCount, (int*)mesh->faces, mesh->facesCount * 3, normals);

    // 2. Generate Handles
    glGenVertexArrays(1, &mesh->gpuMesh->VAO);
    glGenBuffers(1, &mesh->gpuMesh->VBO);
    glGenBuffers(1, &mesh->gpuMesh->EBO);
    glGenBuffers(1, &mesh->gpuMesh->NormalVBO);

    // 3. Bind and Upload
    glBindVertexArray(mesh->gpuMesh->VAO);

    // Vertices
    glBindBuffer(GL_ARRAY_BUFFER, mesh->gpuMesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertexCount * sizeof(Vector3), mesh->vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), (void*)0);
    glEnableVertexAttribArray(0);

    // Normals
    glBindBuffer(GL_ARRAY_BUFFER, mesh->gpuMesh->NormalVBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertexCount * sizeof(Vector3), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), (void*)0);
    glEnableVertexAttribArray(1);

    // Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->gpuMesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->facesCount * 3 * sizeof(int), mesh->faces, GL_STATIC_DRAW);

    glBindVertexArray(0);
    free(normals);

    // return gpu;
}



//
// Calculates normals for every vertex
//
void CalculateNormals(Vector3* vertices, int vCount, int* indices, int iCount, Vector3* outNormals)
{
    // 1. Initialize all normals to 0
    memset(outNormals, 0, vCount * sizeof(Vector3));

    // 2. Loop through every triangle (3 indices at a time)
    for (int i = 0; i < iCount; i += 3) {
        int i0 = indices[i];
        int i1 = indices[i+1];
        int i2 = indices[i+2];

        if (i0 >= vCount || i1 >= vCount || i2 >= vCount || i0 < 0 || i1 < 0 || i2 < 0) 
        {
            printf("--------------------------------------------------\n");
            printf("HEAP CORRUPTION DETECTED!\n");
            printf("You are trying to access Vertex Index %d, %d, %d\n", i0, i1, i2);
            printf("But the Mesh only has %d Vertices allocated.\n", vCount);
            printf("This is the object that is crashing your program.\n");
            printf("--------------------------------------------------\n");
            return; // Stop immediately to save the heap
        }

        Vector3 v0 = vertices[i0];
        Vector3 v1 = vertices[i1];
        Vector3 v2 = vertices[i2];

        // 3. Calculate Face Normal (Cross Product)
        // Edge A = v1 - v0
        Vector3 edge1 = { v1.x - v0.x, v1.y - v0.y, v1.z - v0.z };
        // Edge B = v2 - v0
        Vector3 edge2 = { v2.x - v0.x, v2.y - v0.y, v2.z - v0.z };

        // Cross Product
        Vector3 normal;
        normal.x = edge1.y * edge2.z - edge1.z * edge2.y;
        normal.y = edge1.z * edge2.x - edge1.x * edge2.z;
        normal.z = edge1.x * edge2.y - edge1.y * edge2.x;

        // 4. Add this normal to all 3 vertices (Accumulate)
        // This creates "Smooth Shading".
        outNormals[i0].x += normal.x; outNormals[i0].y += normal.y; outNormals[i0].z += normal.z;
        outNormals[i1].x += normal.x; outNormals[i1].y += normal.y; outNormals[i1].z += normal.z;
        outNormals[i2].x += normal.x; outNormals[i2].y += normal.y; outNormals[i2].z += normal.z;
    }

    // 5. Normalize all results
    for (int i = 0; i < vCount; i++) {
        float len = sqrtf(outNormals[i].x*outNormals[i].x + outNormals[i].y*outNormals[i].y + outNormals[i].z*outNormals[i].z);
        if (len > 0) {
            outNormals[i].x /= len;
            outNormals[i].y /= len;
            outNormals[i].z /= len;
        }
    }
}





//
// Main openGL render function.
// Renders all objects in the specified mode.
//
void RenderGL(SDL_Window* window, Object* objects, int objCount, Camera* cam, unsigned int shaderProgram, int renderMode, bool debugRays, int debugRayCount)
{
    // printf("starting rendering\n");
    // 1. Clear Screen and Depth Buffer Depth buffer is what stops triangles drawing over each other (Z-sorting)
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. Activate Shader
    glUseProgram(shaderProgram);

    // Use specified render mode
    switch(renderMode)
    {
        case 1:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(1.0f);
            break;
        case 2:
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glLineWidth(7.0f);
            break;
        case 0:
        default:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
    }


    // Assign shader variables
    // printf("Assigning shader variables\n");
    float lightDir[3] = {0.0f, -1.0f, -0.2f};
    int lightLoc = glGetUniformLocation(shaderProgram, "lightDir");
    glUniform3fv(lightLoc, 1, lightDir);

    int wfLoc = glGetUniformLocation(shaderProgram, "renderMode");
    glUniform1i(wfLoc, (renderMode > 0) ? 1 : 0);

    // 3. Calculate & Send View Matrix (Camera)
    Matrix4 view = GetViewMatrix(cam);
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (float*)&view);

    // 4. Calculate & Send Projection Matrix (Lens)
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    float aspectRatio = (float)w / (float)h;
    
    // FOV: 1.57 rads (~90 deg), Near: 0.05, Far: 100.0
    Matrix4 proj = Mat4Perspective(1.57f, aspectRatio, 0.05f, 100.0f);
    
    int projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, (float*)&proj);

    // printf("\n----- Starting object drawing -----\n");
    // 5. Draw Objects
    for (int i = 0; i < objCount; i++)
    {
        // printf("Drawing object: %d\n", i);
        Object* obj = &objects[i];

        // printf("Checking if Mesh is valid\n");
        if (!obj->mesh) {
            printf("Objects mesh is NOT valid\n");
            continue;
        }

        if (obj->mesh->gpuMesh == NULL)
        {
            printf("Uploading mesh to GPU\n");
            UploadMeshToGPU(obj->mesh);
        }

        if (obj->mesh->gpuMesh->VAO == 0) {
            printf("Object %d has an invalid VAO (did you use CreateMesh instead of CreateMeshGL?)\n", i);
            continue;
        }

        // printf("Setting more information\n");
        // A. Send Model Matrix (Position/Rotation/Scale)
        Matrix4 model = GetModelMatrix(obj->transform);
        
        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*)&model);

        // B. Send Color
        int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
        glUniform3f(colorLoc, 
                    obj->mesh->color.r / 255.0f, 
                    obj->mesh->color.g / 255.0f, 
                    obj->mesh->color.b / 255.0f);

        // printf("Binding vertex arrays\n");
        // C. Bind Mesh and Draw
        glBindVertexArray(obj->mesh->gpuMesh->VAO);
        
        // printf("Obj %d: VAO: %u, Faces: %d\n\n\n", i, obj->mesh->gpuMesh->VAO, obj->mesh->facesCount);
        // This command replaces your entire rasterizer loop
        glDrawElements(GL_TRIANGLES, obj->mesh->facesCount * 3, GL_UNSIGNED_INT, 0);
    }

    if (debugRays == true && debugRayCount > 0)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        DrawDebugRay(shaderProgram, view, proj, debugRayCount);
    }

    if (renderMode != 0)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // 6. Swap Buffers
    SDL_GL_SwapWindow(window);
}