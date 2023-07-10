// This is also my first C++ Program (Not kidding)!
// Big credits to herbglitch#0024 on discord and YT for helping me get to this point
// Since I used blender, this is Z-up ( I think )
// I did have some help from ChatGPT though. BUT IT WAS ONLY USED FOR WRITING CODE QUICKLY
// To help visualize the object: https://www.geogebra.org/3d?lang=en
// To help summarize what's happening here: https://www.youtube.com/watch?v=hFRlnNci3Rs
// To help understand the math: https://www.youtube.com/playlist?list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab

#include <fstream>
#include <sstream>
#include <cmath> // Uses radians
#include <math.h>
#include <array>
#include <string>
#include <vector>
#include <algorithm>
#include <SDL2/SDL.h>
#include <windows.h>
#include <iostream>

// Desmos example: https://www.desmos.com/calculator/p23yax5qd0
// https://stackoverflow.com/questions/10002918/what-is-the-need-for-normalizing-a-vector
// https://en.wikipedia.org/wiki/Normal_(geometry)
std::array<float, 3> normalizeVector(std::array<float, 3> vector)
{
    float length = std::sqrt(std::pow(vector[0], 2) + std::pow(vector[1], 2) + std::pow(vector[2], 2)); // |V| = sqrt(x*x + y*y + z*z)
    std::array<float, 3> unitVector = {vector[0] / length, vector[1] / length, vector[2] / length}; // V/|V| = (x/|V|, y/|V|, z/|V|)

    return unitVector;
};

// https://en.wikipedia.org/wiki/Dot_product
// dot = •
float dotProduct(std::array<float, 3> a, std::array<float, 3> b)
{
    //     |a1|       |b1|               a1 * b1 +
    // a = |a2| • b = |b2|  dotProduct = a2 * b2 +
    //     |a3|       |b3|               a3 * b3

    return (a[0] * b[0]) +
           (a[1] * b[1]) +
           (a[2] * b[2]);
};

// https://en.wikipedia.org/wiki/Cross_product
// Easier to understand if you watch this: https://www.khanacademy.org/math/linear-algebra/vectors-and-spaces/dot-cross-products/v/linear-algebra-cross-product-introduction
// cross = x
std::array<float, 3> crossProduct(std::array<float, 3> a, std::array<float, 3> b)
{
    std::array<float, 3> crossProduct =
    {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[2] - a[1] * b[0]
    };

    // From the video:
    //    |a1|       |b1|                   |a2 * b3 - a3 * b2|
    //a = |a2| x b = |b2|    crossProduct = |a3 * b1 - a1 * b3|
    //    |a3|       |b3|                   |a1 * b3 - a2 * b1|

    return crossProduct;
};

std::vector<std::array<float, 3>> AngleVectorToRotMatrix(std::array<float, 3> vector) // https://wikimedia.org/api/rest_v1/media/math/render/svg/a8e16f4967571b7a572d1a19f3f6468512f9843e
{
    float x = 2 * M_PI * (vector[0] / 360);
    float y = 2 * M_PI * (vector[1] / 360);
    float z = 2 * M_PI * (vector[2] / 360);

    std::vector<std::array<float, 3>> rotMatrix = {
        {
            std::cos(z) * std::sin(y) * std::sin(x) - std::sin(z) * std::sin(x),
            std::sin(z) * std::sin(y) * std::sin(x) - std::cos(z) * std::sin(x),
            std::cos(y) * std::cos(x)
        },
        {
            std::cos(z) * std::sin(y) * std::sin(x) - std::sin(z) * std::cos(x),
            std::sin(z) * std::sin(y) * std::sin(x) + std::cos(z) * std::cos(x),
            std::cos(y) * std::sin(x)
        },
        {
            std::cos(z) * std::cos(y),
            std::sin(z) * std::cos(y),
            -std::sin(y)
        }
    };

    return rotMatrix;
};

// Matrix multiplication is read right to left
// matrix2, matrix1
std::vector<std::array<float, 3>> matrixMult(std::vector<std::array<float, 3>> m2, std::vector<std::array<float, 3>> m1)
{
    std::vector<std::array<float, 3>> composition =
    {
        {0, 0, 0},
        {0, 0, 0},
        {0, 0, 0}
    };

    for (int i = 0; i < m1.size(); i++)
    {
        for (int j = 0; j < m2[0].size(); j++)
        {
            for (int k = 0; k < m2.size(); k++)
            {
                composition[j][i] += m1[i][k] * m2[k][j];
            }
        }
    };

    return composition;
};

std::vector<std::array<float, 3>> verts;
std::vector<std::array<float, 3>> vertNorms;
std::vector<std::vector<int>> faces; // Indices

// Focal length comparisons: https://cdn-7.nikon-cdn.com/Images/Learn-Explore/Photography-Techniques/2009/Focal-Length/Media/red-barn-sequence.jpg
// Camera diagram: https://digitaltravelcouple.com/wp-content/uploads/2019/04/what-is-focal-length-1-1024x576.jpg?ezimgfmt=rs:767x431/rscb19/ng:webp/ngcb19
float focalLength = 50; // Millimeters

// Apply rotation then offset
std::array<float, 3> objRot = {0, 0, 0}; // Degrees
std::array<float, 3> objOffset = {0, 0, 0};

std::array<float, 3> camRot = {0, 0, 0}; // Orientation
std::array<float, 3> camPos = {0, 0, 5};
std::vector<std::array<float, 3>> camTransform = matrixMult(AngleVectorToRotMatrix(camRot), {{1, 0, camPos[0]}, {0, 1, camPos[1]}, {0, 0, camPos[2]}});

std::array<float, 3> lightDir = {0, 1, 1};

int screenWidth = 1800;
int screenHeight = 900;
float verticalFOV = 60 * (M_PI / 180);

// https://www.youtube.com/watch?v=zMwKYCVNcn0 (Didn't exactly follow this tutorial)
void readOBJ(std::string fileName)
{
    std::stringstream sso;
    std::ifstream file(fileName);
    std::string line;
    std::string prefix;
    std::array<float, 3> tempVector;

    // File open error check
    if (!file.is_open())
    {
        throw "ERROR: Could not open file";
    };

    while (std::getline(file, line))
    {
        // Get the prefix of the line
        sso.clear();
        sso.str(line);
        sso >> prefix;

        if (prefix == "v") // Vertex position
        {
            sso >> tempVector[0] >> tempVector[1] >> tempVector[2];
            
            verts.push_back(tempVector);
        }
        else if (prefix == "vn") // Vertex normal
        {
            sso >> tempVector[0] >> tempVector[1] >> tempVector[2];

            vertNorms.push_back(tempVector);
        }
        else if (prefix == "f") // Face vertex indices
        {
            std::vector<int> indices;
            int index;

            while (sso >> index)
            {
                indices.push_back(index);
                sso >> index >> index; // These are not used
            }

            faces.push_back(indices);
        }
    }
};

void calcFocalLength()
{
    float aspectRatio = screenWidth / screenHeight;
    float horizontalFOV = std::atan(std::tan(verticalFOV / 2) * aspectRatio) * 2;
    
    focalLength = (screenWidth / 2) / std::tan(horizontalFOV / 2);
};

// Project vertex
SDL_FPoint projVert(std::array<float, 3> vert)
{
    // Apply the camera transformation to the vertex
    std::array<float, 3> transformedVert = matrixMult(
        {
            {0, 0, vert[2]},
            {0, vert[1], 0},
            {vert[0], 0, 0}
        },
        camTransform
    )[0];

    // Perform projection calculations
    SDL_FPoint proj =
    {
        focalLength * transformedVert[0] / transformedVert[2], // fl * x / z
        focalLength * transformedVert[1] / transformedVert[2]   // fl * y / z
    };

    // Convert to SDL2 coordinates
    proj =
    {
        (proj.x + screenWidth / 2) / screenWidth,
        1 - (proj.y + screenHeight / 2) / screenHeight
    };

    return proj;
};

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    std::cout << "Running now" << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Error: SDL failed to initialize\nSDL Error: '%s'\n", SDL_GetError());

        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("3D Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        printf("Error: Failed to open window\nSDL Error: '%s'\n", SDL_GetError());

        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    if (!renderer)
    {
        printf("Error: Failed to create renderer\nSDL Error: '%s'\n", SDL_GetError());

        return 1;
    }

    bool running = true;

    readOBJ("C:\\Users\\defga\\OneDrive\\Documents\\3D_Engine_Project\\TestObject2.obj");
    
    verts = matrixMult(verts, AngleVectorToRotMatrix(objRot));
    vertNorms = matrixMult(vertNorms, AngleVectorToRotMatrix(objRot));

    for (int i = 0; i < verts.size(); i++)
    {
        std::array<float, 3> a = verts[i];
        std::array<float, 3> b = vertNorms[i];
        std::array<float, 3> c = objOffset;

        a[0] += c[0];
        a[1] += c[1];
        a[2] += c[2];

        b[0] += c[0];
        b[1] += c[1];
        b[2] += c[2];
    };

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;

            default:
                break;
            }
        }

        SDL_GetWindowSizeInPixels(window, &screenWidth, &screenHeight);

        SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
        SDL_RenderClear(renderer);
        
        calcFocalLength();

        for (int faceI = 0; faceI < faces.size(); faceI++)
        {
            SDL_Vertex verts2D[faces[faceI].size()] = {};

            // For each vertex in the face, draw it on the screen
            for (int i = 0; i < faces[faceI].size(); i++)
            {
                for (int j = 0; j < vertNorms.size(); j++)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        vertNorms[j][k] *= -1;
                    }
                };

                Uint8 v = std::clamp((dotProduct(vertNorms[faceI], lightDir) + 1) / 2 * 255, 0.0f, 255.0f); // v for value = brightness
                SDL_SetRenderDrawColor(renderer, v, v, v, 255);
                SDL_FPoint pos = projVert(verts[faces[faceI][i]]);
                SDL_Color color = {v, v, v, 255};
                SDL_Vertex vert = {pos, color, {1, 1}};

                verts2D[i] = vert;
            };

            SDL_RenderGeometry(renderer, NULL, verts2D, 3, NULL, 0);
        }

        SDL_RenderPresent(renderer);
    }

    return 1;
}
