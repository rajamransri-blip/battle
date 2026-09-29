#include "raylib.h"
#include "raymath.h"
#include "pak.h"

#include <vector>
#include <cmath>

struct Player
{
    Vector3 position{ 0.0f, 0.0f, 0.0f };
    float yaw = 0.0f;
    float health = 100.0f;
    float speed = 7.0f;
    int ammo = 60;
    float shootCooldown = 0.0f;
};

struct Zombie
{
    Vector3 position;
    float health = 100.0f;
    float speed = 1.4f;
    bool alive = true;
};

struct Bullet
{
    Vector3 position;
    Vector3 velocity;
    float life = 2.0f;
    bool alive = true;
};

struct SmartAssets
{
    Model mapModel;
    bool hasMap = false;

    Model playerModel;
    bool hasPlayer = false;

    Model zombieModel;
    bool hasZombie = false;
};

static Vector3 Forward(float yaw)
{
    return Vector3{ std::sinf(yaw), 0.0f, std::cosf(yaw) };
}

static Vector3 Right(float yaw)
{
    return Vector3{ std::cosf(yaw), 0.0f, -std::sinf(yaw) };
}

static void LoadSmartAssets(SmartAssets& assets)
{
    if (FileExistsInPak("maps/map.glb"))
    {
        assets.mapModel = LoadModel("maps/map.glb");
        assets.hasMap = IsModelValid(assets.mapModel);
    }
    if (FileExistsInPak("models/player/player.glb"))
    {
        assets.playerModel = LoadModel("models/player/player.glb");
        assets.hasPlayer = IsModelValid(assets.playerModel);
    }
    if (FileExistsInPak("models/zombies/zombie.glb"))
    {
        assets.zombieModel = LoadModel("models/zombies/zombie.glb");
        assets.hasZombie = IsModelValid(assets.zombieModel);
    }
}

static void UnloadSmartAssets(SmartAssets& assets)
{
    if (assets.hasMap) UnloadModel(assets.mapModel);
    if (assets.hasPlayer) UnloadModel(assets.playerModel);
    if (assets.hasZombie) UnloadModel(assets.zombieModel);
}

static void DrawProceduralTerrain()
{
    DrawSphere(Vector3{ 50.0f, 60.0f, -70.0f }, 10.0f, Color{ 255, 240, 180, 255 });
    DrawPlane(Vector3{ 0, 0, 0 }, Vector2{ 300, 300 }, Color{ 55, 95, 45, 255 });
    DrawCube(Vector3{ 0, 0.02f, 0 }, 9.0f, 0.05f, 280.0f, Color{ 45, 45, 48, 255 });

    for (int x = -40; x <= 40; x += 25)
    {
        for (int z = -40; z <= 40; z += 25)
        {
            if (x == 0 || z == 0) continue;
            Vector3 bp{ (float)x, 2.5f, (float)z };
            DrawCube(bp, 10.0f, 5.0f, 10.0f, Color{ 130, 125, 115, 255 });
            DrawCubeWires(bp, 10.02f, 5.02f, 10.02f, Color{ 60, 55, 50, 255 });
            DrawCube(Vector3{ bp.x, 5.5f, bp.z }, 10.5f, 1.0f, 10.5f, Color{ 90, 40, 30, 255 });
        }
    }

    for (int i = 0; i < 40; ++i)
    {
        float a = (float)i * 0.75f;
        float r = 26.0f + (float)(i % 8) * 5.0f;
        Vector3 tp{ std::cosf(a) * r, 2.0f, std::sinf(a) * r };
        DrawCylinder(tp, 0.35f, 0.45f, 4.0f, 8, Color{ 65, 40, 25, 255 });
        DrawSphere(Vector3{ tp.x, 5.2f, tp.z }, 2.3f, Color{ 35, 95, 40, 255 });
    }
}

static void DrawPlayerEntity(const Player& player, const SmartAssets& assets)
{
    if (assets.hasPlayer)
    {
        DrawModelEx(assets.playerModel, player.position, Vector3{ 0, 1, 0 }, (player.yaw * RAD2DEG) + 180.0f, Vector3{ 1.0f, 1.0f, 1.0f }, WHITE);
        return;
    }

    Vector3 f = Forward(player.yaw);
    Vector3 r = Right(player.yaw);

    DrawCapsule(
        Vector3{ player.position.x, player.position.y + 0.4f, player.position.z },
        Vector3{ player.position.x, player.position.y + 1.4f, player.position.z },
        0.42f, 8, 8, Color{ 35, 55, 85, 255 }
    );
    DrawCube(Vector3{ player.position.x, player.position.y + 1.1f, player.position.z }, 0.6f, 0.65f, 0.45f, Color{ 25, 30, 35, 255 });
    DrawSphere(Vector3{ player.position.x, player.position.y + 1.75f, player.position.z }, 0.32f, Color{ 40, 48, 40, 255 });
    DrawSphere(Vector3{ player.position.x + f.x * 0.15f, player.position.y + 1.68f, player.position.z + f.z * 0.15f }, 0.15f, Color{ 220, 175, 140, 255 });

    Vector3 gunPos = Vector3Add(player.position, Vector3{ r.x * 0.35f + f.x * 0.45f, 1.05f, r.z * 0.35f + f.z * 0.45f });
    DrawCube(gunPos, 0.12f, 0.15f, 0.75f, Color{ 20, 20, 20, 255 });
}

static void DrawZombieEntity(const Zombie& zombie, const SmartAssets& assets, const Player& player)
{
    if (!zombie.alive) return;

    if (assets.hasZombie)
    {
        Vector3 toPlayer = Vector3Subtract(player.position, zombie.position);
        float zYaw = std::atan2f(toPlayer.x, toPlayer.z) * RAD2DEG;
        DrawModelEx(assets.zombieModel, zombie.position, Vector3{ 0, 1, 0 }, zYaw, Vector3{ 1.0f, 1.0f, 1.0f }, WHITE);
        return;
    }

    DrawCapsule(
        Vector3{ zombie.position.x, zombie.position.y + 0.4f, zombie.position.z },
        Vector3{ zombie.position.x, zombie.position.y + 1.4f, zombie.position.z },
        0.42f, 8, 8, Color{ 105, 30, 30, 255 }
    );
    DrawSphere(Vector3{ zombie.position.x, zombie.position.y + 1.75f, zombie.position.z }, 0.34f, Color{ 75, 125, 75, 255 });
}

int main()
{
    SetConfigFlags(FLAG_FULLSCREEN_MODE | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "XBattle");
    SetTargetFPS(60);

    InitPakSystem("game.pak");

    SmartAssets assets;
    LoadSmartAssets(assets);

    Player player;
    std::vector<Zombie> zombies;
    std::vector<Bullet> bullets;

    for (int i = 0; i < 14; ++i)
    {
        float angle = ((float)i / 14.0f) * PI * 2.0f;
        float radius = 16.0f + (float)(i % 4) * 4.0f;
        zombies.push_back({ Vector3{ std::cosf(angle) * radius, 0.0f, std::sinf(angle) * radius } });
    }

    Camera3D camera{};
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 55.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Vector2 prevRightTouchPos{ 0, 0 };
    bool rightTouchActive = false;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        Rectangle fireBtn{ (float)screenW - 140, (float)screenH - 140, 110, 110 };
        Vector2 moveInput{ 0.0f, 0.0f };
        bool fireTriggered = false;

        int touchCount = GetTouchPointCount();
        bool currentRightActive = false;

        for (int i = 0; i < touchCount; ++i)
        {
            Vector2 tPos = GetTouchPosition(i);

            if (CheckCollisionPointRec(tPos, fireBtn))
            {
                fireTriggered = true;
                continue;
            }

            if (tPos.x < (float)screenW * 0.45f)
            {
                Vector2 center{ (float)screenW * 0.18f, (float)screenH * 0.72f };
                Vector2 delta = Vector2Subtract(tPos, center);
                if (Vector2Length(delta) > 15.0f)
                {
                    moveInput = Vector2Normalize(delta);
                }
            }
            else
            {
                currentRightActive = true;
                if (rightTouchActive)
                {
                    float deltaX = tPos.x - prevRightTouchPos.x;
                    player.yaw += deltaX * 0.006f;
                }
                prevRightTouchPos = tPos;
            }
        }
        rightTouchActive = currentRightActive;

        if (IsKeyDown(KEY_W)) moveInput.y -= 1.0f;
        if (IsKeyDown(KEY_S)) moveInput.y += 1.0f;
        if (IsKeyDown(KEY_A)) moveInput.x -= 1.0f;
        if (IsKeyDown(KEY_D)) moveInput.x += 1.0f;
        if (IsKeyDown(KEY_LEFT)) player.yaw -= 2.5f * dt;
        if (IsKeyDown(KEY_RIGHT)) player.yaw += 2.5f * dt;
        if (IsKeyDown(KEY_SPACE)) fireTriggered = true;

        if (Vector2Length(moveInput) > 0.1f)
        {
            Vector2 norm = Vector2Normalize(moveInput);
            Vector3 fwd = Forward(player.yaw);
            Vector3 rgt = Right(player.yaw);
            Vector3 dir = Vector3Add(Vector3Scale(fwd, -norm.y), Vector3Scale(rgt, norm.x));
            player.position = Vector3Add(player.position, Vector3Scale(dir, player.speed * dt));
        }

        if (fireTriggered && player.shootCooldown <= 0.0f && player.ammo > 0)
        {
            Vector3 dir = Forward(player.yaw);
            Vector3 spawn = Vector3Add(player.position, Vector3{ dir.x * 0.8f, 1.1f, dir.z * 0.8f });
            bullets.push_back({ spawn, Vector3Scale(dir, 45.0f) });
            player.ammo--;
            player.shootCooldown = 0.15f;
        }
        if (player.shootCooldown > 0) player.shootCooldown -= dt;

        for (auto& b : bullets)
        {
            if (!b.alive) continue;
            b.position = Vector3Add(b.position, Vector3Scale(b.velocity, dt));
            b.life -= dt;
            if (b.life <= 0) b.alive = false;

            for (auto& z : zombies)
            {
                if (z.alive && Vector3Distance(b.position, z.position) < 1.3f)
                {
                    zombie.health -= 50;
                    b.alive = false;
                    if (zombie.health <= 0) zombie.alive = false;
                    break;
                }
            }
        }

        for (auto& z : zombies)
        {
            if (!z.alive) continue;
            Vector3 dir = Vector3Subtract(player.position, z.position);
            float dist = Vector3Length(dir);
            if (dist > 1.4f)
            {
                dir = Vector3Normalize(dir);
                z.position = Vector3Add(z.position, Vector3Scale(dir, z.speed * dt));
            }
            else
            {
                player.health -= 10.0f * dt;
                if (player.health < 0) player.health = 0;
            }
        }

        // PUBG TPS Over-the-shoulder Camera
        Vector3 fwd = Forward(player.yaw);
        Vector3 rgt = Right(player.yaw);
        camera.position = Vector3Add(
            player.position,
            Vector3{
                -fwd.x * 4.8f + rgt.x * 0.8f,
                2.2f,
                -fwd.z * 4.8f + rgt.z * 0.8f
            }
        );
        camera.target = Vector3Add(player.position, Vector3{ fwd.x * 12.0f, 1.4f, fwd.z * 12.0f });

        BeginDrawing();
        ClearBackground(Color{ 115, 175, 230, 255 });

        BeginMode3D(camera);

        if (assets.hasMap)
        {
            DrawModel(assets.mapModel, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
        }
        else
        {
            DrawProceduralTerrain();
        }

        for (const auto& z : zombies) DrawZombieEntity(z, assets, player);
        DrawPlayerEntity(player, assets);

        for (const auto& b : bullets)
        {
            if (b.alive) DrawSphere(b.position, 0.08f, YELLOW);
        }

        EndMode3D();

        DrawCircle(screenW / 2, screenH / 2, 3.0f, Fade(WHITE, 0.85f));
        DrawCircleLines(screenW / 2, screenH / 2, 8.0f, Fade(BLACK, 0.5f));

        int hudMarginX = 50;
        int hudMarginY = 35;
        DrawRectangle(hudMarginX, hudMarginY, 210, 65, Fade(BLACK, 0.65f));
        DrawText("XBATTLE 3D", hudMarginX + 15, hudMarginY + 10, 20, WHITE);
        DrawText(TextFormat("HP: %d", (int)player.health), hudMarginX + 15, hudMarginY + 36, 18, (player.health > 25 ? GREEN : RED));

        DrawRectangle(screenW - 175, hudMarginY, 125, 45, Fade(BLACK, 0.65f));
        DrawText(TextFormat("AMMO: %d", player.ammo), screenW - 160, hudMarginY + 12, 20, YELLOW);

        Vector2 dpadCenter{ (float)screenW * 0.18f, (float)screenH * 0.72f };
        DrawCircleV(dpadCenter, 65.0f, Fade(WHITE, 0.15f));
        DrawCircleLines(dpadCenter.x, dpadCenter.y, 65.0f, Fade(WHITE, 0.4f));
        DrawCircleV(Vector2Add(dpadCenter, Vector2Scale(moveInput, 38.0f)), 26.0f, Fade(SKYBLUE, 0.6f));

        DrawCircle(fireBtn.x + 55, fireBtn.y + 55, 50, Fade(MAROON, 0.75f));
        DrawCircleLines(fireBtn.x + 55, fireBtn.y + 55, 50, WHITE);
        DrawText("FIRE", fireBtn.x + 33, fireBtn.y + 44, 20, WHITE);

        EndDrawing();
    }

    UnloadSmartAssets(assets);
    ClosePakSystem();
    CloseWindow();
    return 0;
}
