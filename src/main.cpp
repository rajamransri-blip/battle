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
    float speed = 6.0f;
    int ammo = 45;
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
    float life = 2.0f;
    bool alive = true;
};

static Vector3 Forward(float yaw)
{
    return Vector3{
        std::sinf(yaw),
        0.0f,
        std::cosf(yaw)
    };
}

static Vector3 Right(float yaw)
{
    return Vector3{
        std::cosf(yaw),
        0.0f,
        -std::sinf(yaw)
    };
}

static void SpawnZombies(std::vector<Zombie>& zombies)
{
    zombies.clear();
    for (int i = 0; i < 14; ++i)
    {
        float angle = ((float)i / 14.0f) * PI * 2.0f;
        float radius = 14.0f + (float)(i % 4) * 4.5f;

        Zombie z;
        z.position = {
            std::cosf(angle) * radius,
            1.0f,
            std::sinf(angle) * radius
        };
        zombies.push_back(z);
    }
}

static void DrawWorld()
{
    DrawPlane(Vector3{0, 0, 0}, Vector2{140, 140}, Color{68, 98, 64, 255});
    DrawCube(Vector3{0, 0.02f, 0}, 8, 0.05f, 140, Color{60, 60, 60, 255});

    for (int x = -30; x <= 30; x += 20)
    {
        for (int z = -30; z <= 30; z += 20)
        {
            if (x == 0 || z == 0) continue;
            Vector3 p{(float)x, 2.0f, (float)z};
            DrawCube(p, 8, 4, 8, Color{145, 140, 125, 255});
            DrawCube(Vector3{p.x, 4.5f, p.z}, 8.5f, 1.0f, 8.5f, Color{90, 45, 35, 255});
        }
    }

    for (int i = 0; i < 35; ++i)
    {
        float a = (float)i * 0.75f;
        float r = 24.0f + (float)(i % 7) * 4.5f;
        Vector3 p{std::cosf(a) * r, 2.0f, std::sinf(a) * r};

        DrawCylinder(p, 0.25f, 0.35f, 4.0f, 8, Color{75, 45, 30, 255});
        DrawSphere(Vector3{p.x, 5.0f, p.z}, 1.9f, Color{40, 95, 45, 255});
    }
}

static void DrawPlayer(const Player& player)
{
    DrawCapsule(
        Vector3{player.position.x, player.position.y - 0.5f, player.position.z},
        Vector3{player.position.x, player.position.y + 1.0f, player.position.z},
        0.45f, 8, 8, BLUE
    );

    Vector3 f = Forward(player.yaw);
    DrawSphere(
        Vector3{player.position.x + f.x * 0.65f, player.position.y + 0.9f, player.position.z + f.z * 0.65f},
        0.35f, BEIGE
    );
}

static void DrawZombie(const Zombie& zombie)
{
    if (!zombie.alive) return;

    DrawCapsule(
        Vector3{zombie.position.x, 0.5f, zombie.position.z},
        Vector3{zombie.position.x, 2.0f, zombie.position.z},
        0.45f, 8, 8, MAROON
    );

    DrawSphere(
        Vector3{zombie.position.x, 2.35f, zombie.position.z},
        0.42f, Color{100, 150, 100, 255}
    );
}

static void Shoot(Player& player, std::vector<Bullet>& bullets)
{
    if (player.shootCooldown > 0.0f || player.ammo <= 0) return;

    Vector3 direction = Forward(player.yaw);
    Bullet bullet;
    bullet.position = Vector3Add(player.position, Vector3{direction.x * 1.0f, 0.3f, direction.z * 1.0f});
    bullet.velocity = Vector3Scale(direction, 40.0f);
    bullets.push_back(bullet);

    player.ammo--;
    player.shootCooldown = 0.15f;
}

int main()
{
    SetConfigFlags(FLAG_FULLSCREEN_MODE | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "XBattle");
    SetTargetFPS(60);

    Player player;
    std::vector<Zombie> zombies;
    std::vector<Bullet> bullets;
    SpawnZombies(zombies);

    Camera3D camera{};
    camera.position = {0, 5, 8};
    camera.target = {0, 1, 0};
    camera.up = {0, 1, 0};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Vector2 prevRightTouchPos{0, 0};
    bool rightTouchActive = false;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        Rectangle fireBtnRect{ (float)screenW - 140, (float)screenH - 140, 110, 110 };
        Vector2 moveInput{0.0f, 0.0f};
        bool fireTriggered = false;

        int touchCount = GetTouchPointCount();
        bool currentRightTouchActive = false;

        for (int i = 0; i < touchCount; ++i)
        {
            Vector2 tPos = GetTouchPosition(i);

            if (CheckCollisionPointRec(tPos, fireBtnRect))
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
                currentRightTouchActive = true;
                if (rightTouchActive)
                {
                    float deltaX = tPos.x - prevRightTouchPos.x;
                    player.yaw += deltaX * 0.006f;
                }
                prevRightTouchPos = tPos;
            }
        }
        rightTouchActive = currentRightTouchActive;

        if (IsKeyDown(KEY_W)) moveInput.y -= 1.0f;
        if (IsKeyDown(KEY_S)) moveInput.y += 1.0f;
        if (IsKeyDown(KEY_A)) moveInput.x -= 1.0f;
        if (IsKeyDown(KEY_D)) moveInput.x += 1.0f;
        if (IsKeyDown(KEY_LEFT)) player.yaw -= 2.5f * dt;
        if (IsKeyDown(KEY_RIGHT)) player.yaw += 2.5f * dt;
        if (IsKeyDown(KEY_SPACE)) fireTriggered = true;

        if (Vector2Length(moveInput) > 0.1f)
        {
            Vector2 normInput = Vector2Normalize(moveInput);
            Vector3 fwd = Forward(player.yaw);
            Vector3 rgt = Right(player.yaw);

            Vector3 dir = Vector3Add(
                Vector3Scale(fwd, -normInput.y),
                Vector3Scale(rgt, normInput.x)
            );
            player.position = Vector3Add(player.position, Vector3Scale(dir, player.speed * dt));
        }

        if (fireTriggered) Shoot(player, bullets);
        if (player.shootCooldown > 0) player.shootCooldown -= dt;

        for (auto& bullet : bullets)
        {
            if (!bullet.alive) continue;
            bullet.position = Vector3Add(bullet.position, Vector3Scale(bullet.velocity, dt));
            bullet.life -= dt;
            if (bullet.life <= 0) bullet.alive = false;

            for (auto& zombie : zombies)
            {
                if (!zombie.alive) continue;
                if (Vector3Distance(bullet.position, zombie.position) < 1.25f)
                {
                    zombie.health -= 50;
                    bullet.alive = false;
                    if (zombie.health <= 0) zombie.alive = false;
                    break;
                }
            }
        }

        for (auto& zombie : zombies)
        {
            if (!zombie.alive) continue;
            Vector3 direction = Vector3Subtract(player.position, zombie.position);
            float distance = Vector3Length(direction);

            if (distance > 1.4f)
            {
                direction = Vector3Normalize(direction);
                zombie.position = Vector3Add(zombie.position, Vector3Scale(direction, zombie.speed * dt));
            }
            else
            {
                player.health -= 12.0f * dt;
                if (player.health < 0) player.health = 0;
            }
        }

        Vector3 forward = Forward(player.yaw);
        camera.position = Vector3Add(player.position, Vector3{-forward.x * 7.5f, 4.2f, -forward.z * 7.5f});
        camera.target = Vector3Add(player.position, Vector3{0, 1.2f, 0});

        BeginDrawing();
        ClearBackground(Color{105, 165, 220, 255});

        BeginMode3D(camera);
        DrawWorld();
        for (const auto& zombie : zombies) DrawZombie(zombie);
        DrawPlayer(player);
        for (const auto& bullet : bullets)
        {
            if (bullet.alive) DrawSphere(bullet.position, 0.09f, YELLOW);
        }
        EndMode3D();

        DrawRectangle(20, 20, 230, 70, Fade(BLACK, 0.6f));
        DrawText("XBATTLE 3D", 35, 30, 22, WHITE);
        DrawText(TextFormat("HP: %d", (int)player.health), 35, 58, 20, (player.health > 30 ? GREEN : RED));
        DrawText(TextFormat("AMMO: %d", player.ammo), screenW - 160, 25, 22, YELLOW);

        Vector2 leftCenter{ (float)screenW * 0.18f, (float)screenH * 0.72f };
        DrawCircleV(leftCenter, 65.0f, Fade(WHITE, 0.15f));
        DrawCircleLines(leftCenter.x, leftCenter.y, 65.0f, Fade(WHITE, 0.4f));
        DrawCircleV(Vector2Add(leftCenter, Vector2Scale(moveInput, 40.0f)), 28.0f, Fade(SKYBLUE, 0.5f));

        DrawCircle(fireBtnRect.x + 55, fireBtnRect.y + 55, 50, Fade(MAROON, 0.7f));
        DrawCircleLines(fireBtnRect.x + 55, fireBtnRect.y + 55, 50, WHITE);
        DrawText("FIRE", fireBtnRect.x + 32, fireBtnRect.y + 44, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
