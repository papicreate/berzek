// Papiweb desarrollos informaticos
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>

using namespace sf;

const int MAZE_WIDTH = 26;
const int MAZE_HEIGHT = 19;
const float TILE_SIZE = 32.0f;
const float WINDOW_WIDTH = MAZE_WIDTH * TILE_SIZE;
const float WINDOW_HEIGHT = MAZE_HEIGHT * TILE_SIZE;
const float PI = 3.14159265f;
const float PLAYER_SPEED = 150.0f;
const float ROBOT_SPEED = 80.0f;
const float OTTO_SPEED = 250.0f;
const float BULLET_SPEED = 500.0f;
const float BULLET_LIFE = 3.0f;
const float BULLET_RADIUS = 3.0f;
const float OTTO_APPEAR_TIME = 25.0f;
const int START_LIVES = 3;
const int ROBOT_POINTS = 100;
const int CLEAR_BONUS_BASE = 1000;

struct Entity {
    Vector2f pos;
    Vector2f vel;
    float radius;
    Color color;
};

struct Bullet {
    Vector2f pos;
    Vector2f vel;
    bool isPlayerBullet;
    float life;
};

class BerzerkGame {
private:
    RenderWindow window;
    Font font;
    char maze[MAZE_HEIGHT][MAZE_WIDTH];
    Entity player;
    std::vector<Entity> robots;
    Entity otto;
    std::vector<Bullet> bullets;
    float timeElapsed = 0.0f;
    bool ottoActive = false;
    bool gameOver = false;
    bool roomCleared = false;
    int score = 0;
    int lives = START_LIVES;
    int level = 1;
    Vector2f entryDir;  // Not used fully, for simplicity

    Vector2f normalize(const Vector2f& v) {
        float len = std::sqrt(v.x * v.x + v.y * v.y);
        return len > 0.001f ? v / len : Vector2f{0, 0};
    }

    float distance(const Vector2f& a, const Vector2f& b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    bool collidesWall(const Vector2f& pos, float radius) {
        float left = pos.x - radius;
        float right = pos.x + radius;
        float top = pos.y - radius;
        float bottom = pos.y + radius;
        int tx1 = static_cast<int>(std::floor(left / TILE_SIZE));
        int tx2 = static_cast<int>(std::floor(right / TILE_SIZE));
        int ty1 = static_cast<int>(std::floor(top / TILE_SIZE));
        int ty2 = static_cast<int>(std::floor(bottom / TILE_SIZE));
        for (int ty = ty1; ty <= ty2; ++ty) {
            for (int tx = tx1; tx <= tx2; ++tx) {
                if (tx >= 0 && tx < MAZE_WIDTH && ty >= 0 && ty < MAZE_HEIGHT && maze[ty][tx] == '#') {
                    return true;
                }
            }
        }
        return false;
    }

    void moveEntity(Entity& entity, float dt, bool checkWalls = true) {
        Vector2f deltaPos = entity.vel * dt;
        Vector2f oldPos = entity.pos;

        // Separate axis movement for collision
        entity.pos.x += deltaPos.x;
        if (checkWalls && collidesWall(entity.pos, entity.radius)) {
            entity.pos.x = oldPos.x;
            entity.vel.x *= -0.5f;
        }

        oldPos.x = entity.pos.x;
        entity.pos.y += deltaPos.y;
        if (checkWalls && collidesWall(entity.pos, entity.radius)) {
            entity.pos.y = oldPos.y;
            entity.vel.y *= -0.5f;
        }

        // Friction/damping
        entity.vel *= 0.9f;
    }

    void generateMaze() {
        // Fill with walls
        for (int y = 0; y < MAZE_HEIGHT; ++y) {
            for (int x = 0; x < MAZE_WIDTH; ++x) {
                maze[y][x] = '#';
            }
        }
        // Open inner area sparsely
        for (int y = 1; y < MAZE_HEIGHT - 1; ++y) {
            for (int x = 1; x < MAZE_WIDTH - 1; ++x) {
                if (rand() % 8 == 0) {
                    maze[y][x] = ' ';
                }
            }
        }
        // Add 2-4 doors on borders
        int numDoors = 2 + (rand() % 3);
        for (int d = 0; d < numDoors; ++d) {
            int side = rand() % 4;
            if (side == 0 && rand() % 2) {  // Left
                int y = 1 + rand() % (MAZE_HEIGHT - 2);
                maze[y][0] = ' ';
            } else if (side == 1) {  // Right
                int y = 1 + rand() % (MAZE_HEIGHT - 2);
                maze[y][MAZE_WIDTH - 1] = ' ';
            } else if (side == 2) {  // Top
                int x = 1 + rand() % (MAZE_WIDTH - 2);
                maze[0][x] = ' ';
            } else {  // Bottom
                int x = 1 + rand() % (MAZE_WIDTH - 2);
                maze[MAZE_HEIGHT - 1][x] = ' ';
            }
        }
    }

    void placePlayer() {
        // Place near random door for authenticity
        int side = rand() % 4;
        float px, py;
        if (side == 0) {  // Left
            py = TILE_SIZE * 1.5f + rand() % static_cast<int>((MAZE_HEIGHT - 3) * TILE_SIZE);
            px = TILE_SIZE * 1.5f;
        } else if (side == 1) {  // Right
            py = TILE_SIZE * 1.5f + rand() % static_cast<int>((MAZE_HEIGHT - 3) * TILE_SIZE);
            px = (MAZE_WIDTH - 1.5f) * TILE_SIZE;
        } else if (side == 2) {  // Top
            px = TILE_SIZE * 1.5f + rand() % static_cast<int>((MAZE_WIDTH - 3) * TILE_SIZE);
            py = TILE_SIZE * 1.5f;
        } else {  // Bottom
            px = TILE_SIZE * 1.5f + rand() % static_cast<int>((MAZE_WIDTH - 3) * TILE_SIZE);
            py = (MAZE_HEIGHT - 1.5f) * TILE_SIZE;
        }
        player.pos = {px, py};
        player.vel = {0, 0};
    }

    void spawnRobots() {
        robots.clear();
        int numRobots = std::min(10, 2 + level);
        for (int i = 0; i < numRobots; ++i) {
            Entity rob;
            rob.radius = 12.0f;
            rob.color = Color::Red;
            bool valid = false;
            for (int attempts = 0; attempts < 50; ++attempts) {
                int x = 1 + rand() % (MAZE_WIDTH - 2);
                int y = 1 + rand() % (MAZE_HEIGHT - 2);
                if (maze[y][x] == ' ') {
                    rob.pos = {x * TILE_SIZE + TILE_SIZE / 2.0f, y * TILE_SIZE + TILE_SIZE / 2.0f};
                    if (!collidesWall(rob.pos, rob.radius) && distance(rob.pos, player.pos) > 3 * TILE_SIZE) {
                        valid = true;
                        break;
                    }
                }
            }
            if (valid) {
                rob.vel = {0, 0};
                robots.push_back(rob);
            }
        }
    }

    void shoot(const Vector2f& dir) {
        Vector2f dirNorm = normalize(dir);
        Bullet b;
        b.pos = player.pos + dirNorm * player.radius;
        b.vel = dirNorm * BULLET_SPEED;
        b.isPlayerBullet = true;
        b.life = BULLET_LIFE;
        bullets.push_back(b);
    }

    void robotShoot(const Entity& robot) {
        Vector2f delta = player.pos - robot.pos;
        // Simplified alignment: horizontal, vertical, or diagonal
        float absDx = std::fabs(delta.x);
        float absDy = std::fabs(delta.y);
        if (absDx < TILE_SIZE * 0.5f || absDy < TILE_SIZE * 0.5f || (std::fabs(absDx - absDy) < TILE_SIZE * 0.2f && absDx < 6 * TILE_SIZE)) {
            Vector2f dirNorm = normalize(delta);
            Bullet b;
            b.pos = robot.pos + dirNorm * robot.radius;
            b.vel = dirNorm * BULLET_SPEED * 0.7f;  // Slower
            b.isPlayerBullet = false;
            b.life = BULLET_LIFE * 0.8f;
            bullets.push_back(b);
        }
    }

    void updatePlayer(float dt) {
        moveEntity(player, dt, true);
        if (collidesWall(player.pos, player.radius)) {
            lives--;
            if (lives <= 0) {
                gameOver = true;
            } else {
                placePlayer();
            }
        }
    }

    void updateRobots(float dt) {
        for (auto& robot : robots) {
            // AI: try direct to player, fallback axes
            Vector2f toPlayer = normalize(player.pos - robot.pos);
            Vector2f testPosX = robot.pos + Vector2f(toPlayer.x * TILE_SIZE * 1.5f, 0);
            Vector2f testPosY = robot.pos + Vector2f(0, toPlayer.y * TILE_SIZE * 1.5f);
            Vector2f testPosDiag = robot.pos + toPlayer * TILE_SIZE * 1.5f;

            bool canDiag = !collidesWall(testPosDiag, robot.radius);
            bool canX = !collidesWall(testPosX, robot.radius);
            bool canY = !collidesWall(testPosY, robot.radius);

            if (canDiag) {
                robot.vel = toPlayer * ROBOT_SPEED;
            } else if (canX && canY) {
                robot.vel = toPlayer * ROBOT_SPEED * 0.7f;
            } else if (canX) {
                robot.vel = {toPlayer.x * ROBOT_SPEED, 0};
            } else if (canY) {
                robot.vel = {0, toPlayer.y * ROBOT_SPEED};
            } else {
                robot.vel = {0, 0};
            }

            // Occasional shoot
            static float shootAccum = 0.0f;
            shootAccum += dt;
            if (shootAccum > 1.0f / level) {
                robotShoot(robot);
                shootAccum = 0.0f;
            }

            moveEntity(robot, dt, true);
        }
    }

    void updateBullets(float dt) {
        std::vector<Bullet> newBullets;
        for (auto& bullet : bullets) {
            bullet.pos += bullet.vel * dt;
            bullet.life -= dt;
            if (bullet.life > 0.0f && !collidesWall(bullet.pos, BULLET_RADIUS)) {
                newBullets.push_back(bullet);
            }
        }
        bullets = std::move(newBullets);
    }

    void updateOtto(float dt) {
        Vector2f toPlayer = normalize(player.pos - otto.pos);
        otto.vel = toPlayer * OTTO_SPEED;
        moveEntity(otto, dt, false);  // No wall collision
    }

    void checkCollisions() {
        // Player bullets hit robots
        std::vector<Bullet> newBullets;
        for (auto& bullet : bullets) {
            if (bullet.isPlayerBullet) {
                bool hit = false;
                for (auto it = robots.begin(); it != robots.end(); ) {
                    if (distance(bullet.pos, it->pos) < it->radius + BULLET_RADIUS) {
                        score += ROBOT_POINTS;
                        it = robots.erase(it);
                        hit = true;
                        roomCleared = false;  // Reset until all clear
                    } else {
                        ++it;
                    }
                }
                if (!hit) {
                    newBullets.push_back(bullet);
                }
            } else {
                newBullets.push_back(bullet);
            }
        }
        bullets = std::move(newBullets);

        if (robots.empty() && !roomCleared) {
            score += CLEAR_BONUS_BASE * level;
            roomCleared = true;
        }

        // Enemy bullets hit player
        for (auto it = bullets.begin(); it != bullets.end(); ) {
            if (!it->isPlayerBullet && distance(it->pos, player.pos) < player.radius + BULLET_RADIUS) {
                lives--;
                if (lives <= 0) {
                    gameOver = true;
                }
                it = bullets.erase(it);
            } else {
                ++it;
            }
        }

        // Entity collisions
        for (auto& robot : robots) {
            if (distance(player.pos, robot.pos) < player.radius + robot.radius) {
                lives--;
                if (lives <= 0) {
                    gameOver = true;
                } else {
                    placePlayer();
                }
                break;
            }
        }
        if (ottoActive && distance(player.pos, otto.pos) < player.radius + otto.radius) {
            lives--;
            if (lives <= 0) {
                gameOver = true;
            } else {
                placePlayer();
            }
        }
    }

    void checkExit() {
        int px = static_cast<int>(std::floor(player.pos.x / TILE_SIZE));
        int py = static_cast<int>(std::floor(player.pos.y / TILE_SIZE));
        if (player.pos.x < TILE_SIZE * 1.5f && px >= 0 && maze[py][0] == ' ') {
            nextRoom();
        } else if (player.pos.x > WINDOW_WIDTH - TILE_SIZE * 1.5f && px < MAZE_WIDTH && maze[py][MAZE_WIDTH - 1] == ' ') {
            nextRoom();
        } else if (player.pos.y < TILE_SIZE * 1.5f && py >= 0 && maze[0][px] == ' ') {
            nextRoom();
        } else if (player.pos.y > WINDOW_HEIGHT - TILE_SIZE * 1.5f && py < MAZE_HEIGHT && maze[MAZE_HEIGHT - 1][px] == ' ') {
            nextRoom();
        }
    }

    void nextRoom() {
        generateMaze();
        placePlayer();
        spawnRobots();
        bullets.clear();
        ottoActive = false;
        timeElapsed = 0.0f;
        roomCleared = false;
        level++;
    }

    void activateOtto() {
        ottoActive = true;
        otto.pos = {WINDOW_WIDTH / 2.0f, TILE_SIZE * 2.0f};
        otto.vel = {0, 0};
        otto.radius = 16.0f;
        otto.color = Color::Yellow;
    }

    void drawMaze() {
        RectangleShape tile(Vector2f(TILE_SIZE, TILE_SIZE));
        tile.setFillColor(Color(64, 64, 64));
        for (int y = 0; y < MAZE_HEIGHT; ++y) {
            for (int x = 0; x < MAZE_WIDTH; ++x) {
                if (maze[y][x] == '#') {
                    tile.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                    window.draw(tile);
                }
            }
        }
    }

    void drawCircle(const Vector2f& pos, float radius, const Color& color) {
        CircleShape circle(radius);
        circle.setPosition(pos - Vector2f(radius, radius));
        circle.setFillColor(color);
        circle.setOutlineColor(Color::Black);
        circle.setOutlineThickness(1.0f);
        window.draw(circle);
    }

    void drawUI() {
        Text text;
        text.setFont(font);
        text.setCharacterSize(24);
        text.setFillColor(Color::White);

        text.setString("Score: " + std::to_string(score));
        text.setPosition(10, 10);
        window.draw(text);

        text.setString("Lives: " + std::to_string(lives));
        text.setPosition(10, 40);
        window.draw(text);

        text.setString("Level: " + std::to_string(level));
        text.setPosition(10, 70);
        window.draw(text);

        text.setString("Robots: " + std::to_string(robots.size()));
        text.setPosition(10, 100);
        window.draw(text);

        text.setString("Time: " + std::to_string(static_cast<int>(timeElapsed)));
        text.setPosition(10, 130);
        window.draw(text);

        if (roomCleared) {
            text.setString("ROOM CLEARED! Exit through a door!");
            text.setPosition(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2);
            text.setCharacterSize(32);
            window.draw(text);
        }

        if (gameOver) {
            text.setString("GAME OVER\nPress R to restart, Q to quit");
            text.setPosition(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2);
            text.setCharacterSize(32);
            window.draw(text);
        }
    }

public:
    BerzerkGame() : window(VideoMode(static_cast<unsigned int>(WINDOW_WIDTH), static_cast<unsigned int>(WINDOW_HEIGHT)), "Berzerk SFML Clone") {
        window.setFramerateLimit(60);
        srand(static_cast<unsigned int>(time(nullptr)));
        if (!font.loadFromFile("font.ttf")) {
            std::cerr << "Error: Download a TTF font (e.g., Liberation Sans from Google Fonts), rename to 'font.ttf' and place in executable directory!" << std::endl;
            window.close();
        }
        player.radius = 12.0f;
        player.color = Color::Green;
        nextRoom();
    }

    void run() {
        Clock clock;
        while (window.isOpen()) {
            float dt = clock.restart().asSeconds();

            // Handle input
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed) {
                    window.close();
                }
                if (event.type == Event::KeyPressed) {
                    if (event.key.code == Keyboard::Q || event.key.code == Keyboard::Escape) {
                        window.close();
                    }
                    if (gameOver && event.key.code == Keyboard::R) {
                        restart();
                    }
                    if (!gameOver) {
                        if (event.key.code == Keyboard::Up) {
                            shoot({0, -1});
                        } else if (event.key.code == Keyboard::Down) {
                            shoot({0, 1});
                        } else if (event.key.code == Keyboard::Left) {
                            shoot({-1, 0});
                        } else if (event.key.code == Keyboard::Right) {
                            shoot({1, 0});
                        }
                    }
                }
            }

            // Continuous movement
            if (!gameOver) {
                player.vel = {0, 0};
                if (Keyboard::isKeyPressed(Keyboard::W)) player.vel.y -= PLAYER_SPEED;
                if (Keyboard::isKeyPressed(Keyboard::S)) player.vel.y += PLAYER_SPEED;
                if (Keyboard::isKeyPressed(Keyboard::A)) player.vel.x -= PLAYER_SPEED;
                if (Keyboard::isKeyPressed(Keyboard::D)) player.vel.x += PLAYER_SPEED;
                if (player.vel.x != 0 && player.vel.y != 0) {
                    player.vel *= 0.7071f;  // Normalize diagonal
                }

                updatePlayer(dt);
                updateRobots(dt);
                updateBullets(dt);
                if (ottoActive) {
                    updateOtto(dt);
                }
                checkCollisions();
                timeElapsed += dt;
                if (timeElapsed > OTTO_APPEAR_TIME && !ottoActive) {
                    activateOtto();
                }
                checkExit();
            }

            // Render
            window.clear(Color::Black);
            drawMaze();
            for (const auto& robot : robots) {
                drawCircle(robot.pos, robot.radius, robot.color);
            }
            drawCircle(player.pos, player.radius, player.color);
            if (ottoActive) {
                drawCircle(otto.pos, otto.radius, otto.color);
            }
            for (const auto& bullet : bullets) {
                RectangleShape bShape;
                float len = bullet.isPlayerBullet ? 24.0f : 12.0f;
                bShape.setSize({4.0f, len});
                float angle = std::atan2(bullet.vel.y, bullet.vel.x) * 180.0f / PI;
                bShape.setRotation(angle);
                bShape.setOrigin(2.0f, len / 2.0f);
                bShape.setPosition(bullet.pos);
                bShape.setFillColor(bullet.isPlayerBullet ? Color::White : Color::Cyan);
                window.draw(bShape);
            }
            drawUI();
            window.display();
        }
    }

    void restart() {
        score = 0;
        lives = START_LIVES;
        level = 1;
        gameOver = false;
        nextRoom();
    }
};

int main() {
    BerzerkGame game;
    game.run();
    return 0;
}