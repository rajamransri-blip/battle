#include "raylib.h"
#include "raymath.h"

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(1280, 720, "XReal3D");

    SetTargetFPS(60);

    Camera3D camera{};
    camera.position = Vector3{6.0f, 4.0f, 6.0f};
    camera.target = Vector3{0.0f, 1.0f, 0.0f};
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (IsKeyDown(KEY_W))
            camera.position.z -= 4.0f * dt;

        if (IsKeyDown(KEY_S))
            camera.position.z += 4.0f * dt;

        if (IsKeyDown(KEY_A))
            camera.position.x -= 4.0f * dt;

        if (IsKeyDown(KEY_D))
            camera.position.x += 4.0f * dt;

        BeginDrawing();

        ClearBackground(Color{105, 160, 210, 255});

        BeginMode3D(camera);

        DrawPlane(
            Vector3{0, 0, 0},
            Vector2{100, 100},
            Color{75, 100, 70, 255}
        );

        DrawCube(
            Vector3{0, 1, 0},
            2.0f,
            2.0f,
            2.0f,
            Color{150, 150, 150, 255}
        );

        DrawCubeWires(
            Vector3{0, 1, 0},
            2.0f,
            2.0f,
            2.0f,
            BLACK
        );

        DrawGrid(100, 1.0f);

        EndMode3D();

        DrawText(
            "XReal3D - 3D C++ Engine",
            25,
            25,
            28,
            WHITE
        );

        DrawText(
            "W A S D - Camera",
            25,
            60,
            20,
            WHITE
        );

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
