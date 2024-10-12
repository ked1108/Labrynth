#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

class Cell;

class Set {
public:
    std::vector<Cell*> Cells;
};

class Cell {
public:
    Set* set = nullptr;
    bool hasRightWall = true;
    bool hasBottomWall = true;
};

class Maze {
public:
    void GenerateMaze(int width, int height, unsigned int seed);
    void DrawMaze3D(float cellSize);


private:
    int width, height;
    std::vector<Set*> sets;
    std::vector<Cell> row;
    std::vector<std::vector<Cell>> maze;

    std::random_device rd;
    std::mt19937 rng{ rd() };
    std::uniform_int_distribution<int> dist{ 0, 64 };

    bool CreateWall();
    void InitSets();
    void CreateRightWalls();
    void CreateBottomWalls();
    void PrepareNextRow();
    void WriteRowIntoMaze(int rowIdx);
};

bool Maze::CreateWall() {
    return dist(rng) > 32;
}

void Maze::InitSets() {
    for (auto& cell : row) {
        if (cell.set == nullptr) {
            Set* newSet = new Set();
            cell.set = newSet;
            newSet->Cells.push_back(&cell);
            sets.push_back(newSet);
        }
    }
}

void Maze::CreateRightWalls() {
    for (size_t i = 0; i < row.size() - 1; ++i) {
        if (CreateWall()) {
            row[i].hasRightWall = true;
        } else if (row[i].set != row[i + 1].set) {
            row[i].hasRightWall = false;
            // Merge sets
            Set* rightSet = row[i + 1].set;
            for (auto& cell : rightSet->Cells) {
                row[i].set->Cells.push_back(cell);
                cell->set = row[i].set;
            }
            rightSet->Cells.clear();
        } else {
            row[i].hasRightWall = true;
        }
    }
    row.back().hasRightWall = true;
}

void Maze::CreateBottomWalls() {
    for (auto& set : sets) {
        if (!set->Cells.empty()) {
            std::vector<int> indices;
            int pathsDown = std::uniform_int_distribution<int>{1, static_cast<int>(set->Cells.size())}(rng);
            while (indices.size() < static_cast<size_t>(pathsDown)) {
                int index = std::uniform_int_distribution<int>{0, static_cast<int>(set->Cells.size()) - 1}(rng);
                if (std::find(indices.begin(), indices.end(), index) == indices.end()) {
                    indices.push_back(index);
                }
            }

            for (size_t i = 0; i < set->Cells.size(); ++i) {
                if (std::find(indices.begin(), indices.end(), static_cast<int>(i)) == indices.end()) {
                    set->Cells[i]->hasBottomWall = true;
                } else {
                    set->Cells[i]->hasBottomWall = false;
                }
            }
        }
    }
}

void Maze::PrepareNextRow() {
    for (auto& cell : row) {
        cell.hasRightWall = false;
        if (cell.hasBottomWall) {
            auto it = std::find(cell.set->Cells.begin(), cell.set->Cells.end(), &cell);
            if (it != cell.set->Cells.end()) {
                cell.set->Cells.erase(it);
            }
            cell.set = nullptr;
            cell.hasBottomWall = false;
        }
    }
}

void Maze::WriteRowIntoMaze(int rowIdx) {
    for (int i = 0; i < width; ++i) {
        maze[rowIdx][i] = row[i];
    }
}

void Maze::GenerateMaze(int width, int height, unsigned int seed) {
    this->width = width;
    this->height = height;

    rng.seed(seed);

    maze.resize(height, std::vector<Cell>(width));
    row.resize(width);

    for (int y = 0; y < height; ++y) {
        InitSets();

        if (y == height - 1) {
            for (size_t i = 0; i < row.size() - 1; ++i) {
                if (row[i].set != row[i + 1].set) {
                    row[i].hasRightWall = false;
                }
            }
            for (auto& cell : row) {
                cell.hasBottomWall = true;
            }
            WriteRowIntoMaze(y);
            continue;
        }

        CreateRightWalls();
        CreateBottomWalls();
        WriteRowIntoMaze(y);
        PrepareNextRow();
    }
}
//
// void Maze::DrawMaze3D(float cellSize) {
//
// }
//
//
//
// void Maze::DrawMaze3D(float cellSize) {
//     // Iterate through each cell in the maze
//     for (int y = 0; y < height; ++y) {
//         for (int x = 0; x < width; ++x) {
//             Cell& cell = maze[y][x];
//
//             // Calculate the position of the current cell
//             Vector3 position = (Vector3){ x * cellSize, 0, y * cellSize };
//
//             // Draw the right wall if it exists
//             if (cell.hasRightWall) {
//                 // Right wall position (aligned with the right edge of the cell)
//                 Vector3 rightWallPos = (Vector3){ position.x + cellSize / 2, cellSize / 2, position.z };
//                 // Draw a thin vertical wall (cube)
//                 DrawCube(rightWallPos, 0.1f, cellSize, cellSize, GRAY); // Wall thickness of 0.1
//             }
//
//             // Draw the bottom wall if it exists
//             if (cell.hasBottomWall) {
//                 // Bottom wall position (aligned with the bottom edge of the cell)
//                 Vector3 bottomWallPos = (Vector3){ position.x, cellSize / 2, position.z + cellSize / 2 };
//                 // Draw a thin horizontal wall (cube)
//                 DrawCube(bottomWallPos, cellSize, cellSize, 0.1f, GRAY); // Wall thickness of 0.1
//             }
//
//             // Optionally, you can draw a floor for the cell (if you want it)
//             Vector3 floorPos = (Vector3){ position.x, 0, position.z };
//             DrawCube(floorPos, cellSize, 0.1f, cellSize, LIGHTGRAY); // Thin floor at the bottom of the cell
//         }
//     }
//
//     // Optionally draw outer walls for the maze (not necessary if the maze is closed properly)
//     // Draw the outer walls (top, bottom, left, and right borders)
//     for (int x = 0; x < width; ++x) {
//         // Top outer wall
//         Vector3 topWallPos = (Vector3){ x * cellSize, cellSize / 2, -cellSize / 2 };
//         DrawCube(topWallPos, cellSize, cellSize, 0.1f, GRAY);
//
//         // Bottom outer wall
//         Vector3 bottomWallPos = (Vector3){ x * cellSize, cellSize / 2, height * cellSize - cellSize / 2 };
//         DrawCube(bottomWallPos, cellSize, cellSize, 0.1f, GRAY);
//     }
//
//     for (int y = 0; y < height; ++y) {
//         // Left outer wall
//         Vector3 leftWallPos = (Vector3){ -cellSize / 2, cellSize / 2, y * cellSize };
//         DrawCube(leftWallPos, 0.1f, cellSize, cellSize, GRAY);
//
//         // Right outer wall
//         Vector3 rightWallPos = (Vector3){ width * cellSize - cellSize / 2, cellSize / 2, y * cellSize };
//         DrawCube(rightWallPos, 0.1f, cellSize, cellSize, GRAY);
//     }
// }

void Maze::DrawMaze3D(float cellSize) {
    // Generate a reusable wall mesh using GenMeshPlane
    Mesh wallMesh = GenMeshPlane(cellSize, cellSize, 1, 1);  // A flat wall with thickness 0.1f
    Mesh floorMesh = GenMeshPlane(cellSize, cellSize, 1, 1);  // A flat plane for the floor

    // Load the mesh into a model to apply transformations
    Model wallModel = LoadModelFromMesh(wallMesh);
    Model floorModel = LoadModelFromMesh(floorMesh);

    // Iterate through each cell in the maze
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Cell& cell = maze[y][x];

            // Calculate the base position of the current cell
            Vector3 position = (Vector3){ x * cellSize, 0, y * cellSize };
            rlDisableBackfaceCulling();
            // Draw the right wall if it exists
            if (cell.hasRightWall) {
                // Right wall should be vertical along the x-axis, positioned on the right edge of the cell
                Matrix wallTransform = MatrixRotateX(PI / 2);  // Rotate the plane 90 degrees around the X-axis to stand it upright
                wallTransform = MatrixMultiply(wallTransform, MatrixTranslate(position.x + cellSize / 2, cellSize / 2, position.z));
                wallModel.transform = wallTransform;  // Apply rotation and translation for the right wall
                DrawModel(wallModel, (Vector3){ 0, 0, 0 }, 1.0f, GRAY);
            }

            // Draw the bottom wall if it exists
            if (cell.hasBottomWall) {
                // Bottom wall should be horizontal along the z-axis, positioned on the bottom edge of the cell
                Matrix wallTransform = MatrixRotateX(PI / 2);  // Rotate the plane to stand it upright
                wallTransform = MatrixMultiply(wallTransform, MatrixRotateY(PI / 2));  // Rotate around Y-axis to place it along the Z-axis
                wallTransform = MatrixMultiply(wallTransform, MatrixTranslate(position.x, cellSize / 2, position.z + cellSize / 2));
                wallModel.transform = wallTransform;
                DrawModel(wallModel, (Vector3){ 0, 0, 0 }, 1.0f, GRAY);
            }

            // Optionally draw the floor for each cell
            floorModel.transform = MatrixTranslate(position.x, -0.05f, position.z);  // Slightly below 0 for the floor
            DrawModel(floorModel, (Vector3){ 0, 0, 0 }, 1.0f, LIGHTGRAY);
        }
    }

    // Clean up
    UnloadModel(wallModel);  // Unload the wall model (and the associated mesh)
    UnloadModel(floorModel);  // Unload the floor model
}


int main() {
    // Initialize raylib
    InitWindow(1080, 720, "Maze Game");

    // Define cell size and wall thickness
    int cellSize = 5;
    Maze maze;
    maze.GenerateMaze(100, 100, 32);

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type


    // / Perspective camera

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        // Update camera controls (optional)
        UpdateCamera(&camera, CAMERA_FREE);
        // Begin drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Begin 3D mode
        BeginMode3D(camera);
        maze.DrawMaze3D(cellSize); // Draw the maze in 3D
        EndMode3D();

        DrawFPS(10, 10); // Draw FPS
        EndDrawing();
    }

    CloseWindow(); // Close window and OpenGL context
    return 0;
}

