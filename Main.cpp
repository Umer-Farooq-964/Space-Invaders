#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PLAYER_SPEED = 5;
const int BULLET_SPEED = 8;
const int ENEMY_SPEED = 2;

const Vector2f ENEMY_SIZE(40.0f, 40.0f); // Target size for enemies
const Vector2f PLAYER_SIZE(50.0f, 50.0f); // Target size for player

struct Bullet {
    RectangleShape shape;
    bool active;
};

struct Enemy {
    Sprite sprite;
    bool active;
    vector<Bullet> bullets; // Enemy's bullets
    Clock shootClock; // Timer for shooting
    int shootDelay; // Random shooting delay for each enemy
};

void resetGame(Sprite& playerSprite, vector<Bullet>& bullets, vector<Enemy>& enemies, Texture& enemyTexture, int enemyRows, int enemyColumns) {
    // Reset player position
    playerSprite.setPosition(WINDOW_WIDTH / 2.0f - playerSprite.getGlobalBounds().width / 2.0f, WINDOW_HEIGHT - PLAYER_SIZE.y - 20.0f);

    // Clear bullets
    bullets.clear();

    // Reset enemies
    enemies.clear();
    const float enemySpacing = 10.0f;
    for (int i = 0; i < enemyRows; ++i) {
        for (int j = 0; j < enemyColumns; ++j) {
            Enemy enemy;
            enemy.sprite.setTexture(enemyTexture);

            // Scale enemy sprite to fit the target size
            FloatRect enemyBounds = enemy.sprite.getGlobalBounds();
            float scaleX = ENEMY_SIZE.x / enemyBounds.width;
            float scaleY = ENEMY_SIZE.y / enemyBounds.height;
            enemy.sprite.setScale(scaleX, scaleY);

            enemy.sprite.setPosition(j * (ENEMY_SIZE.x + enemySpacing) + 50, i * (ENEMY_SIZE.y + enemySpacing) + 50);
            enemy.active = true;

            // Randomly set shoot delay between 1 to 4 seconds for each enemy
            enemy.shootDelay = rand() % 20000 + 1000; // Delay between 1000ms (1s) to 4000ms (4s)

            enemies.push_back(enemy);
        }
    }
}

int main() {
    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Space Invaders");
    window.setFramerateLimit(60);

    // Load textures
    Texture playerTexture, enemyTexture, backgroundTexture;
    if (!playerTexture.loadFromFile("Assets/player.png") || !enemyTexture.loadFromFile("Assets/enemy.png") || !backgroundTexture.loadFromFile("Assets/bg.png")) {
        return -1; // Exit if textures fail to load
    }

    // Background setup
    Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTexture);

    // Scale background to fit the window
    FloatRect backgroundBounds = backgroundSprite.getGlobalBounds();
    float backgroundScaleX = WINDOW_WIDTH / backgroundBounds.width;
    float backgroundScaleY = WINDOW_HEIGHT / backgroundBounds.height;
    backgroundSprite.setScale(backgroundScaleX, backgroundScaleY);

    // Player setup
    Sprite playerSprite;
    playerSprite.setTexture(playerTexture);

    // Scale player sprite to fit the target size
    FloatRect playerBounds = playerSprite.getGlobalBounds();
    float playerScaleX = PLAYER_SIZE.x / playerBounds.width;
    float playerScaleY = PLAYER_SIZE.y / playerBounds.height;
    playerSprite.setScale(playerScaleX, playerScaleY);

    playerSprite.setPosition(WINDOW_WIDTH / 2.0f - playerSprite.getGlobalBounds().width / 2.0f, WINDOW_HEIGHT - PLAYER_SIZE.y - 20.0f);

    // Bullets
    vector<Bullet> bullets;

    // Enemies
    vector<Enemy> enemies;
    const int enemyRows = 3;
    const int enemyColumns = 10;

    // Initialize game state
    resetGame(playerSprite, bullets, enemies, enemyTexture, enemyRows, enemyColumns);

    // Enemy direction
    float enemyDirection = ENEMY_SPEED;

    Music titleMusic, inGameMusic;
    if (!titleMusic.openFromFile("Assets/title_music.mp3") || !inGameMusic.openFromFile("Assets/in_game_music.mp3")) {
        return -1;
    }
    titleMusic.setLoop(true);
    inGameMusic.setLoop(true);
    titleMusic.play();

    // Title Screen
    Font font1, font2;

    if (!font1.loadFromFile("Assets/ElectronPulse.ttf")) {
        return -1; // Exit if font fails to load
    }
    if (!font2.loadFromFile("Assets/Pixel.ttf")) {
        return -1; // Exit if font fails to load
    }

    // Title text setup
    Text titleText;
    titleText.setFont(font1);
    titleText.setString("Space Invaders");
    titleText.setCharacterSize(70);
    titleText.setFillColor(Color::White);
    titleText.setPosition(WINDOW_WIDTH / 2 - titleText.getGlobalBounds().width / 2, WINDOW_HEIGHT / 3);

    // "Press SPACE to Start" text setup
    Text pressStartText;
    pressStartText.setFont(font2);
    pressStartText.setString("Press SPACE to Start");
    pressStartText.setCharacterSize(20);
    pressStartText.setFillColor(Color::White);
    pressStartText.setPosition(WINDOW_WIDTH / 2 - pressStartText.getGlobalBounds().width / 2, WINDOW_HEIGHT / 2 + 50);

    Text madeby;
    madeby.setFont(font2);
    madeby.setString("Made by M.Umer Farooq & Amna Mudasser");
    madeby.setCharacterSize(20);
    madeby.setFillColor(Color::White);
    madeby.setPosition(WINDOW_WIDTH / 2 - madeby.getGlobalBounds().width / 2, WINDOW_HEIGHT / 2 + 200);

    // Lose screen setup
    Text loseText;
    loseText.setFont(font2);
    loseText.setString("         You Lose!\nPress R to play again");
    loseText.setCharacterSize(30);
    loseText.setFillColor(Color::White);
    loseText.setPosition(WINDOW_WIDTH / 2 - loseText.getGlobalBounds().width / 2, WINDOW_HEIGHT / 3);

    // Game state flags
    bool gameStarted = false;
    bool gameOver = false;
    bool youWin = false;

    // Win screen setup
    Text winText;
    winText.setFont(font1);
    winText.setString("You Win!");
    winText.setCharacterSize(70);
    winText.setFillColor(Color::White);
    winText.setPosition(WINDOW_WIDTH / 2 - winText.getGlobalBounds().width / 2, WINDOW_HEIGHT / 3);

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }

        if (!gameStarted) {
            // Title screen logic
            if (Keyboard::isKeyPressed(Keyboard::Space)) {
                gameStarted = true; // Start the game when space is pressed
                titleMusic.stop(); // Stop title music
                inGameMusic.play(); // Start game music
            }

            window.clear();
            window.draw(backgroundSprite);
            window.draw(titleText); // Draw the title
            window.draw(pressStartText); // Draw the "Press SPACE" text
            window.draw(madeby);
            window.display();
            continue;
        }

        // Player movement
        if (Keyboard::isKeyPressed(Keyboard::Left) && playerSprite.getPosition().x > 0) {
            playerSprite.move(-PLAYER_SPEED, 0);
        }
        if (Keyboard::isKeyPressed(Keyboard::Right) && playerSprite.getPosition().x + playerSprite.getGlobalBounds().width < WINDOW_WIDTH) {
            playerSprite.move(PLAYER_SPEED, 0);
        }

        // Shoot bullets
        if (Keyboard::isKeyPressed(Keyboard::Space)) {
            static Clock shootClock;
            if (shootClock.getElapsedTime().asMilliseconds() > 500) {
                Bullet bullet;
                bullet.shape.setSize(Vector2f(5.0f, 10.0f));
                bullet.shape.setFillColor(Color::White);
                bullet.shape.setPosition(
                    playerSprite.getPosition().x + playerSprite.getGlobalBounds().width / 2 - bullet.shape.getSize().x / 2,
                    playerSprite.getPosition().y
                );
                bullet.active = true;
                bullets.push_back(bullet);
                shootClock.restart();
            }
        }

        // Update bullets
        for (auto& bullet : bullets) {
            if (bullet.active) {
                bullet.shape.move(0, -BULLET_SPEED);
                if (bullet.shape.getPosition().y < 0) {
                    bullet.active = false;
                }
            }
        }

        // Update enemies
        bool changeDirection = false;
        bool allEnemiesDestroyed = true;
        for (auto& enemy : enemies) {
            if (enemy.active) {
                allEnemiesDestroyed = false;
                enemy.sprite.move(enemyDirection, 0);

                // Check if enemy hits the screen edge for direction change
                if (enemy.sprite.getPosition().x <= 0 || enemy.sprite.getPosition().x + enemy.sprite.getGlobalBounds().width >= WINDOW_WIDTH) {
                    changeDirection = true;
                }

                // Check if enemy reaches the bottom of the screen
                if (enemy.sprite.getPosition().y + enemy.sprite.getGlobalBounds().height >= playerSprite.getPosition().y) {
                    gameOver = true;
                    break;
                }

                // Random enemy shooting based on shootClock
                if (enemy.shootClock.getElapsedTime().asMilliseconds() > enemy.shootDelay) { // Shoot based on random delay
                    Bullet enemyBullet;
                    enemyBullet.shape.setSize(Vector2f(5.0f, 10.0f));
                    enemyBullet.shape.setFillColor(Color::Red);
                    enemyBullet.shape.setPosition(
                        enemy.sprite.getPosition().x + enemy.sprite.getGlobalBounds().width / 2 - enemyBullet.shape.getSize().x / 2,
                        enemy.sprite.getPosition().y + enemy.sprite.getGlobalBounds().height
                    );
                    enemyBullet.active = true;
                    enemy.bullets.push_back(enemyBullet);
                    enemy.shootClock.restart();
                    // Reassign new random shoot delay for next shot
                    enemy.shootDelay = rand() % 10000 + 5000;
                }

                // Move enemy's bullets
                for (auto& enemyBullet : enemy.bullets) {
                    if (enemyBullet.active) {
                        enemyBullet.shape.move(0, BULLET_SPEED);
                        // Check if enemy bullet hits the player
                        if (enemyBullet.shape.getGlobalBounds().intersects(playerSprite.getGlobalBounds())) {
                            gameOver = true;
                        }
                    }
                }
            }
        }

        // Handle game over
        if (gameOver) {
            window.clear();
            window.draw(backgroundSprite);
            window.draw(loseText); // Draw the "You Lose" message
            window.display();

            if (Keyboard::isKeyPressed(Keyboard::R)) {
                // Restart the game when "R" is pressed
                resetGame(playerSprite, bullets, enemies, enemyTexture, enemyRows, enemyColumns);
                enemyDirection = ENEMY_SPEED; // Reset direction
                gameOver = false; // Reset game over flag
            }

            continue;
        }

        // Handle "You Win"
        if (allEnemiesDestroyed) {
            youWin = true;
        }

        if (youWin) {
            window.clear();
            window.draw(backgroundSprite);
            window.draw(winText); // Draw the "You Win" message
            window.display();
            // Wait for a few seconds before restarting
            sleep(seconds(3)); // Adjust the duration for how long the "You Win" message shows

            // Restart the game
            resetGame(playerSprite, bullets, enemies, enemyTexture, enemyRows, enemyColumns);
            enemyDirection = ENEMY_SPEED; // Reset direction
            youWin = false;
        }

        if (changeDirection) {
            enemyDirection = -enemyDirection;
            for (auto& enemy : enemies) {
                enemy.sprite.move(0, ENEMY_SIZE.y); // Move enemies down
            }
        }

        // Check collisions for player bullets and enemies
        for (auto& bullet : bullets) {
            if (bullet.active) {
                for (auto& enemy : enemies) {
                    if (enemy.active && bullet.shape.getGlobalBounds().intersects(enemy.sprite.getGlobalBounds())) {
                        bullet.active = false; // Deactivate player bullet on collision
                        enemy.active = false;  // Deactivate enemy on collision
                        break;
                    }
                }
            }
        }

        // Check for player bullet collisions with enemy bullets
        for (auto& enemy : enemies) {
            for (auto& enemyBullet : enemy.bullets) {
                if (enemyBullet.active && enemyBullet.shape.getGlobalBounds().intersects(playerSprite.getGlobalBounds())) {
                    gameOver = true;  // Trigger game over if enemy bullet hits player
                    break;
                }
            }
        }

        // Remove inactive bullets
        bullets.erase(remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) { return !b.active; }), bullets.end());

        // Remove inactive enemy bullets
        for (auto& enemy : enemies) {
            enemy.bullets.erase(remove_if(enemy.bullets.begin(), enemy.bullets.end(), [](const Bullet& b) { return !b.active; }), enemy.bullets.end());
        }

        // Rendering
        window.clear();

        // Draw background
        window.draw(backgroundSprite);

        // Draw player
        window.draw(playerSprite);

        // Draw bullets
        for (const auto& bullet : bullets) {
            if (bullet.active) {
                window.draw(bullet.shape);
            }
        }

        // Draw enemy bullets
        for (const auto& enemy : enemies) {
            for (const auto& enemyBullet : enemy.bullets) {
                if (enemyBullet.active) {
                    window.draw(enemyBullet.shape);
                }
            }
        }

        // Draw enemies
        for (const auto& enemy : enemies) {
            if (enemy.active) {
                window.draw(enemy.sprite);
            }
        }

        window.display();
    }

    return 0;
}
