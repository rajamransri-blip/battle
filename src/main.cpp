#include "raylib.h"
#include "raymath.h"

#include <vector>
#include <cmath>
#include <cstdlib>

struct Player
{
    Vector3 position{0.0f, 1.0f, 0.0f};
    float yaw = 0.0f;
    float health = 100.0f;
    float speed = 6.5f;
    int ammo = 50;
    float shootCooldown = 0.0f;
};

struct Zombie
{
    Vector3 position;
    float health = 100.0f;
    float speed = 1.35f;
    bool alive = true;
};

struct Bullet
{
    Vector3 position;
    Vector3 velocity;
    float life = 2.5f;
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
    // Auto-detect and Load map.glb
    if (FileExists("maps/map.glb"))
    {
        assets.mapModel = LoadModel("maps/map.glb");
        assets.hasMap = IsModelValid(assets.mapModel);
    }

    // Auto-detect and Load player.glb
    if (FileExists("models/player/player.glb"))
    {
        assets.playerModel = LoadModel("models/player/player.glb");
        assets.hasPlayer = IsModelValid(assets.playerModel);
    }

    // Auto-detect and Load zombie.glb
    if (FileExists("models/zombies/zombie.glb"))
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

static void DrawRealisticSkyAndGround()
{
    // Sun
    DrawSphere(Vector3{ 60.0f, 45.0f, -80.0f }, 12.0f, Color{ 255, 245, 180, 255 });
    DrawSphere(Vector3{ 60.0f, 45.0f, -80.0f }, 16.0f, Color{ 255, 230, 140, 60 });

    // Distant Mountain Ranges (Horizon)
    for (int i = 0; i < 20; ++i)
    {
        float angle = ((float)i / 20.0f) * PI * 2.0f;
        float dist = 135.0f;
        Vector3 pos{ std::cosf(angle) * dist, 0.0f, std::sinf(angle) * dist };
        DrawCylinder(pos, 0.0f, 22.0f + (float)(i % 5) * 4.0f, 25.0f + (float)(i % 3) * 6.0f, 4, Color{ 65, 80, 95, 255 });
    }

    // Ground Terrain
    DrawPlane(Vector3{ 0, 0, 0 }, Vector2{ 280, 280 }, Color{ 70, 105, 62, 255 });

    // Tactical Road
    DrawCube(Vector3{ 0, 0.02f, 0 }, 9.0f, 0.04f, 260.0f, Color{ 55, 55, 58, 255 });
    for (float z = -120.0f; z < 120.0f; z += 12.0f)
    {
        DrawCube(Vector3{ 0, 0.05f, z }, 0.6f, 0.02f, 5.0f, Color{ 240, 230, 160, 255 });
    }

    // Environment Structures
    for (int x = -40; x <= 40; x += 25)
    {
        for (int z = -40; z <= 40; z += 25)
        {
            if (x == 0 || z == 0) continue;
            Vector3 bp{ (float)x, 2.5f, (float)z };
            DrawCube(bp, 10.0f, 5.0f, 10.0f, Color{ 140, 135, 120, 255 });
            DrawCube(Vector3{ bp.x, 5.5f, bp.z }, 10.5f, 1.2f, 10.5f, Color{ 100, 50, 40, 255 });
        }
    }

    // Trees
    for (int i = 0; i < 45; ++i)
    {
        float a = (float)i * 0.72f;
        float r = 28.0f + (float)(i % 8) * 5.0f;
        Vector3 tp{ std::cosf(a) * r, 2.0f, std::sinf(a) * r };
        DrawCylinder(tp, 0.3f, 0.45f, 4.5f, 8, Color{ 75, 45, 30, 255 });
        DrawSphere(Vector3{ tp.x, 5.5f, tp.z }, 2.4f, Color{ 45, 110, 48, 255 });
    }
}

static void SpawnZombies(std::vector<Zombie>& zombies)
{
    zombies.clear();
    for (int i = 0; i < 15; ++i)
    {
        float angle = ((float)i / 15.0f) * PI * 2.0f;
        float radius = 15.0f + (float)(i % 5) * 4.5f;
        Zombie z;
        z.position = { std::cosf(angle) * radius, 1.0f, std::sinf(angle) * radius };
        zombies.push_back(z);
    }
}

static void DrawPlayerEntity(const Player& player, const SmartAssets& assets)
{
    if (assets.hasPlayer)
    {
        Vector3 rotAxis{ 0.0f, 1.0f, 0.0f };
        float rotAngle = (player.yaw * RAD2DEG) + 180.0f;
        DrawModelEx(assets.playerModel, player.position, rotAxis, rotAngle, Vector3{ 1.0f, 1.0f, 1.0f }, WHITE);
    }
    else
    {
        // Realistic procedural soldier body
        DrawCapsule(
            Vector3{ player.position.x, player.position.y - 0.5f, player.position.z },
            Vector3{ player.position.x, player.position.y + 1.0f, player.position.z },
            0.45f, 8, 8, Color{ 40, 60, 80, 255 }
        );
        Vector3 f = Forward(player.yaw);
        // Head & Helmet
        DrawSphere(Vector3{ player.position.x, player.position.y + 1.15f, player.position.z }, 0.38f, Color{ 30, 40, 30, 255 });
        DrawSphere(Vector3{ player.position.x + f.x * 0.2f, player.position.y + 1.10f, player.position.z + f.z * 0.2f }, 0.16f, Color{ 230, 185, 150, 255 });
        // Weapon
        DrawCube(Vector3{ player.position.x + f.x * 0.65f, player.position.y + 0.7f, player.position.z + f.z * 0.65f }, 0.12f, 0.18f, 0.7f, DARKGRAY);
    }
}

static void DrawZombieEntity(const Zombie& zombie, const SmartAssets& assets, const Player& player)
{
    if (!zombie.alive) return;

    if (assets.hasZombie)
    {
        Vector3 toPlayer = Vector3Subtract(player.position, zombie.position);
        float zYaw = std::atan2f(toPlayer.x, toPlayer.z) * RAD2DEG;
        DrawModelEx(assets.zombieModel, zombie.position, Vector3{ 0, 1, 0 }, zYaw, Vector3{ 1.0f, 1.0f, 1.0f }, WHITE);
    }
    else
    {
        DrawCapsule(
            Vector3{ zombie.position.x, 0.5f, zombie.position.z },
            Vector3{ zombie.position.x, 2.0f, zombie.position.z },
            0.45f, 8, 8, Color{ 110, 35, 35, 255 }
        );
        DrawSphere(Vector3{ zombie.position.x, 2.35f, zombie.position.z }, 0.40f, Color{ 90, 140, 90, 255 });
    }
}

static void Shoot(Player& player, std::vector<Bullet>& bullets)
{
    if (player.shootCooldown > 0.0f || player.ammo <= 0) return;

    Vector3 dir = Forward(player.yaw);
    Bullet b;
    b.position = Vector3Add(player.position, Vector3{ dir.x * 0.9f, 0.5f, dir.z * 0.9f });
    b.velocity = Vector3Scale(dir, 42.0f);
    bullets.push_back(b);

    player.ammo--;
    player.shootCooldown = 0.14f;
}

int main()
{
    SetConfigFlags(FLAG_FULLSCREEN_MODE | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "XBattle");
    SetTargetFPS(60);

    SmartAssets assets;
    LoadSmartAssets(assets);

    Player player;
    std::vector<Zombie> zombies;
    std::vector<Bullet> bullets;
    SpawnZombies(zombies);

    Camera3D camera{};
    camera.position = { 0, 5, 8 };
    camera.target = { 0, 1, 0 };
    camera.up = { 0, 1, 0 };
    camera.fovy = 62.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Vector2 prevRightTouchPos{ 0, 0 };
    bool rightTouchActive = false;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        Rectangle fireBtn{ (float)screenW - 145, (float)screenH - 145, 115, 115 };
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
            else if (tPos.x >= (float)screenW * 0.45f)
            {
                currentRightActive = true;
                if (rightTouchActive)
                {
                    float deltaX = tPos.x - prevRightTouchPos.x;
                    player.yaw += deltaX * 0.0065f;
                }
                prevRightTouchPos = tPos;
            }
        }
        rightTouchActive = currentRightActive;

        // Keyboard Fallbacks
        if (IsKeyDown(KEY_W)) moveInput.y -= 1.0f;
        if (IsKeyDown(KEY_S)) moveInput.y += 1.0f;
        if (IsKeyDown(KEY_A)) moveInput.x -= 1.0f;
        if (IsKeyDown(KEY_D)) moveInput.x += 1.0f;
        if (IsKeyDown(KEY_LEFT)) player.yaw -= 2.6f * dt;
        if (IsKeyDown(KEY_RIGHT)) player.yaw += 2.6f * dt;
        if (IsKeyDown(KEY_SPACE)) fireTriggered = true;

        if (Vector2Length(moveInput) > 0.1f)
        {
            Vector2 norm = Vector2Normalize(moveInput);
            Vector3 fwd = Forward(player.yaw);
            Vector3 rgt = Right(player.yaw);
            Vector3 dir = Vector3Add(Vector3Scale(fwd, -norm.y), Vector3Scale(rgt, norm.x));
            player.position = Vector3Add(player.position, Vector3Scale(dir, player.speed * dt));
        }

        if (fireTriggered) Shoot(player, bullets);
        if (player.shootCooldown > 0) player.shootCooldown -= dt;

        // Update Bullets
        for (auto& bullet : bullets)
        {
            if (!bullet.alive) continue;
            bullet.position = Vector3Add(bullet.position, Vector3Scale(bullet.velocity, dt));
            bullet.life -= dt;
            if (bullet.life <= 0) bullet.alive = false;

            for (auto& zombie : zombies)
            {
                if (!zombie.alive) continue;
                if (Vector3Distance(bullet.position, zombie.position) < 1.35f)
                {
                    zombie.health -= 50;
                    bullet.alive = false;
                    if (zombie.health <= 0) zombie.alive = false;
                    break;
                }
            }
        }

        // Update Zombies
        for (auto& zombie : zombies)
        {
            if (!zombie.alive) continue;
            Vector3 dir = Vector3Subtract(player.position, zombie.position);
            float dist = Vector3Length(dir);
            if (dist > 1.35f)
            {
                dir = Vector3Normalize(dir);
                zombie.position = Vector3Add(zombie.position, Vector3Scale(dir, zombie.speed * dt));
            }
            else
            {
                player.health -= 12.0f * dt;
                if (player.health < 0) player.health = 0;
            }
        }

        // Smooth Third-Person Camera
        Vector3 fwd = Forward(player.yaw);
        camera.position = Vector3Add(player.position, Vector3{ -fwd.x * 7.5f, 4.2f, -fwd.z * 7.5f });
        camera.target = Vector3Add(player.position, Vector3{ 0, 1.2f, 0 });

        // Render
        BeginDrawing();
        ClearBackground(Color{ 135, 195, 235, 255 }); // Realistic atmospheric blue

        BeginMode3D(camera);

        // Smart Map Rendering
        if (assets.hasMap)
        {
            DrawModel(assets.mapModel, Vector3{ 0, 0, 0 }, 1.0f, WHITE);
        }
        else
        {
            DrawRealisticSkyAndGround();
        }

        // Entities
        for (const auto& zombie : zombies) DrawZombieEntity(zombie, assets, player);
        DrawPlayerEntity(player, assets);

        for (const auto& bullet : bullets)
        {
            if (bullet.alive)
            {
                DrawSphere(bullet.position, 0.12f, YELLOW);
            }
        }

        EndMode3D();

        // Real-time HUD
        DrawRectangle(25, 25, 240, 75, Fade(BLACK, 0.65f));
        DrawText("XBATTLE 3D", 40, 34, 22, WHITE);
        DrawText(TextFormat("HP: %d", (int)player.health), 40, 62, 20, (player.health > 30 ? GREEN : RED));
        DrawText(TextFormat("AMMO: %d", player.ammo), screenW - 170, 30, 22, YELLOW);

        // Controls Overlay
        Vector2 dpadCenter{ (float)screenW * 0.18f, (float)screenH * 0.72f };
        DrawCircleV(dpadCenter, 65.0f, Fade(WHITE, 0.15f));
        DrawCircleLines(dpadCenter.x, dpadCenter.y, 65.0f, Fade(WHITE, 0.45f));
        DrawCircleV(Vector2Add(dpadCenter, Vector2Scale(moveInput, 38.0f)), 26.0f, Fade(SKYBLUE, 0.6f));

        DrawCircle(fireBtn.x + 57, fireBtn.y + 57, 52, Fade(MAROON, 0.75f));
        DrawCircleLines(fireBtn.x + 57, fireBtn.y + 57, 52, WHITE);
        DrawText("FIRE", fireBtn.x + 35, fireBtn.y + 46, 20, WHITE);

        EndDrawing();
    }

    UnloadSmartAssets(assets);
    CloseWindow();
    return 0;
}
