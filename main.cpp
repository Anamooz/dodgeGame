#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>
#include <algorithm>

const int FONT_CHAR_WIDTH  = 8;
const int FONT_CHAR_HEIGHT = 10;
const int FONT_COLUMNS = 10;
const int FONT_ROW = 3;
struct Bee{
    sf::Sprite sprite;
    int currentFrame = 0;
    float animTimer = 0.f;
    float fallSpeed = 150.f;

    Bee(const sf::Texture& texture)
        : sprite(texture)   // construct sprite here
    {
        sprite.setTextureRect(sf::IntRect{{0, 0}, {36, 34}});
        sprite.setOrigin({18.f, 17.f}); // half of 36x34
    }
};

bool loadTexture(sf::Texture& texture, const std::string& path);
void clampToScreen(sf::Sprite& sprite, const sf::RenderWindow& window);
void drawScore(sf::RenderWindow& window, const sf::Texture& textTexture, int score, sf::Vector2f position, float scale = 3.f);

int main(){
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Dodge Game");
    window.setFramerateLimit(60);

    float scoreTimer = 0.f;
    int score = 0;
    const int scoreRate = 200;

    sf::Texture idleTexture;
    sf::Texture runTexture;
    sf::Texture hitTexture;
    sf::Texture terrainTexture;
    sf::Texture skyTexture;
    sf::Texture beeTexture;
    sf::Texture textTexture;
    sf::Texture heartTexture;

    if(!loadTexture(idleTexture, "Assets/Main Characters/Pink Man/Idle (32x32).png")){
        std::cout << "Failed to load idle texture\n";
        return -1;
    }

    if(!loadTexture(runTexture, "Assets/Main Characters/Pink Man/Run (32x32).png")){
        std::cout << "Failed to load run texture\n";
        return -1;
    }

    if(!loadTexture(hitTexture, "Assets/Main Characters/Pink Man/Hit (32x32).png")){
        std::cout << "Failed to load hit texture\n";
        return -1;
    }

    if(!loadTexture(terrainTexture, "Assets/Terrain/Terrain (16x16).png")){
        std::cout << "Failed to load terrain texture\n";
        return -1;
    } 
    
    if(!loadTexture(skyTexture, "Assets/Background/orig_big.png")){
        std::cout << "Failed to load sky texture\n";
        return -1;
    } 

    if(!loadTexture(beeTexture, "Assets/Enemies/Bee/Idle (36x34).png")){
        std::cout << "Failed to load bee texture\n";
        return -1;
    }

    if(!loadTexture(textTexture, "Assets/Menu/Text/Text (White) (8x10).png")){
        std::cout << "Failed to load text texture\n";
        return -1;
    }

    if(!loadTexture(heartTexture, "Assets/Menu/Buttons/Heart (32x32).png")){
        std::cout << "Failed to load heart texture\n";
        return -1;
    }

    // ------Terrain setup--------
    sf::Sprite terrainTile(terrainTexture);
    terrainTile.setTextureRect(sf::IntRect{ {0,0},{16,16} }); 

    sf::Sprite skySprite(skyTexture);
    sf::Vector2u texSize = skyTexture.getSize(); // Scale sky to fill window
    sf::Vector2u winSize = window.getSize();

    skySprite.setScale(sf::Vector2f(
        static_cast<float>(winSize.x) / texSize.x,
        static_cast<float>(winSize.y) / texSize.y
    ));

    skySprite.setPosition(sf::Vector2f(0.f, 0.f));

    const int tileSize = 16;
    const int terrainRows = 2;
    const float tileSizeFloat = 16.f;
    const float playerHalf = 16.f; // 32x32 sprite
    float grassTopY = window.getSize().y - tileSizeFloat * 2;

    
    int windowWidth = window.getSize().x;
    int windowHeight = window.getSize().y;

    int tileWidth = windowWidth / tileSize + 1; // +1 in case of rounding, tiles fitting horizontally
    int tileHeight = windowHeight / tileSize + 1; // tiles fitting vertically
    
    std::vector<sf::Sprite> terrainRow; // Pre-create terrain tiles in a vector
    terrainRow.reserve(tileWidth); // avoids reallocations
    sf::IntRect dirtRect {{144, 0}, {tileSize, tileSize}};
    sf::IntRect grassRect {{128, 0}, {tileSize, tileSize}};

    for(int i = 0; i < terrainRows; ++i){
        for (int j = 0; j < tileWidth; ++j){

            if(i == 0)
                terrainTile.setTextureRect(dirtRect);
            else
                terrainTile.setTextureRect(grassRect);

            // Set position using explicit floats to avoid narrowing warnings
            terrainTile.setPosition(sf::Vector2f(
                static_cast<float>(j * tileSize),
                static_cast<float>(windowHeight - tileSize * (i + 1))
            ));

            terrainRow.push_back(terrainTile);
        }
    }   

    // ------Sprite player setup--------
    sf::Sprite sprite(idleTexture); //start idle texture
    sprite.setTextureRect(sf::IntRect{ {0, 0}, {32, 32} });
    sprite.setOrigin({16.f, 16.f});
    sprite.setPosition({400.f, grassTopY - playerHalf});


                               // Animation variables
    const int idleFrames = 11; // Number of frames in idle animation
    const int runFrames  = 12; // Number of frames in run animation

    int currentFrame = 0;
    float frameTimer = 0.f;
    float frameSpeed = 0.10f;

    const sf::Texture* currentTexture = &idleTexture; // Keep track of current texture pointer

    // ------ Hearts Setup------
    const int maxLives = 5;
    int lives = maxLives;

    sf::Sprite heartSprite(heartTexture);

    const float heartSize = 32.f;   
    const float heartSpacing = 8.f;

    // ------ Invincibility Frames ------
    bool invincible = false;
    float invincibilityTimer = 0.f;
    const float invincibilityDuration = 2.5f;

        // Blink control
    float blinkTimer = 0.f;
    const float blinkInterval = 0.15f;
    bool visible = true;


    // ------Player hit animation setup--------
    bool isHit = false; 

    const int hitFrames = 7;
    const float hitAnimSpeed = 0.08f;
    float hitTimer = 0.f;
    int hitFrame = 0;


    // ------Movement setup--------
    float speed = 200.f; // pixels per second
    bool facingLeft = false;

    // ------Bee setup--------
    std::vector<Bee> bees;
    sf::Clock spawnClock;

    const float spawnInterval = 1.f;

    const int beeFrameCount = 6;
    const int beeFrameWidth  = 36;
    const int beeFrameHeight = 34;
    const float beeAnimSpeed = 0.1f; // seconds per frame

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> xDist(
    beeFrameWidth / 2,
    window.getSize().x - beeFrameWidth / 2);

    sf::Clock clock;

    // ------Game loop--------
    while (window.isOpen()){
        while (auto event = window.pollEvent()){
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        float dt = clock.restart().asSeconds(); // Time since last frame

        scoreTimer += dt;
        score += static_cast<int>(scoreRate * dt);

    // ------Spawn bees------
        if (spawnClock.getElapsedTime().asSeconds() >= spawnInterval){
            spawnClock.restart();

            Bee bee(beeTexture);

            float randomX = static_cast<float>(xDist(rng));
            bee.sprite.setPosition({randomX, -beeFrameHeight / 2.f});

            bees.push_back(bee);
        }

    // ------ Invincibility Update ------
        if (invincible) {
            invincibilityTimer += dt;
            blinkTimer += dt;

            if (blinkTimer >= blinkInterval) {
                blinkTimer = 0.f;
                visible = !visible;
                sprite.setColor(visible ? sf::Color::White : sf::Color(255, 255, 255, 0));
            }

            if (invincibilityTimer >= invincibilityDuration) {
                invincible = false;
                visible = true;
                sprite.setColor(sf::Color::White);
            }
    }
    
    // --- Hit Detection ---
        if (!invincible && !isHit) {
            for (auto it = bees.begin(); it != bees.end(); ++it) {
                if (it->sprite
                        .getGlobalBounds()
                        .findIntersection(sprite.getGlobalBounds())
                        .has_value())
                {
                    // Lose a heart
                    lives = std::max(0, lives - 1);

                    invincible = true;
                    invincibilityTimer = 0.f;
                    blinkTimer = 0.f;
                    visible = false;

                    isHit = true;
                    hitFrame = 0;
                    hitTimer = 0.f;

                    sprite.setTexture(hitTexture, false);
                    sprite.setTextureRect(sf::IntRect{{0, 0}, {32, 32}});

                    // Remove the bee that hit the player
                    bees.erase(it);

                    break;
                }
            }
        }

    // --- Hit Animation ---
        if (isHit){
            hitTimer += dt;

            if (hitTimer >= hitAnimSpeed) {
                hitTimer = 0.f;
                hitFrame++;

                if (hitFrame >= hitFrames){
                    // Hit animation finished
                    isHit = false;

                    // Return to idle state
                    sprite.setTexture(idleTexture, false);
                    currentTexture = &idleTexture;
                    currentFrame = 0;
                    frameTimer = 0.f;

                    sprite.setTextureRect(sf::IntRect{{0, 0}, {32, 32}});
                }
                else
                {
                    sprite.setTextureRect(sf::IntRect{
                        {hitFrame * 32, 0},
                        {32, 32}
                    });
                }
            }
        }


    // --- Movement Input ---
        bool moving = false;

        // Move left
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            sprite.move({-speed * dt, 0.f});
            facingLeft = true;
            moving = true;
        }

        // Move right
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            sprite.move({speed * dt, 0.f});
            facingLeft = false;
            moving = true;
        }

        clampToScreen(sprite, window); 

        // Flip sprite by using scale
        sprite.setScale({facingLeft ? -1.f : 1.f, 1.f});
        
        // --- Animation ---
        if(!isHit) { // Ensures hit animation isn't interrupted
    
            frameTimer += dt;

            // Switching texture if state changes
            if (moving && currentTexture != &runTexture){
                sprite.setTexture(runTexture, false); //keep false so the rect isn't reset to full png
                currentTexture = &runTexture;
                currentFrame = 0;
                frameTimer = 0.f;
                sprite.setTextureRect(sf::IntRect{{0,0},{32,32}}); //keeps rect correct even if the above false is removed
            }
            else if (!moving && currentTexture != &idleTexture)
            {
                sprite.setTexture(idleTexture, false);
                currentTexture = &idleTexture;
                currentFrame = 0;
                frameTimer = 0.f;
                sprite.setTextureRect(sf::IntRect{{0,0},{32,32}});
            }

            // Advance frame timer
            if (frameTimer >= frameSpeed){
                frameTimer = 0.f;
                if (moving)
                    currentFrame = (currentFrame + 1) % runFrames;
                else
                    currentFrame = (currentFrame + 1) % idleFrames;

                sprite.setTextureRect(sf::IntRect{{currentFrame*32,0},{32,32}});
            }
        }

    // --- Bee Animation and Movement ---

        for (auto& bee : bees){
            // Falling movement
            bee.sprite.move({0.f, bee.fallSpeed * dt});

            // Animation
            bee.animTimer += dt;
            if (bee.animTimer >= beeAnimSpeed){
                bee.animTimer = 0.f;
                bee.currentFrame = (bee.currentFrame + 1) % beeFrameCount;

                bee.sprite.setTextureRect(sf::IntRect{
                    {bee.currentFrame * beeFrameWidth, 0},
                    {beeFrameWidth, beeFrameHeight}
                });
            }
        }
        // Remove bees once it hits grass (top of terrain)
        bees.erase(
            std::remove_if(bees.begin(), bees.end(),
                [&](const Bee& bee)
                {
                    float beeTop =
                        bee.sprite.getPosition().y - beeFrameHeight / 2.f;

                    return beeTop >= grassTopY;
                }),
            bees.end()
        );


        // -------Rendering---------
        window.clear();

        // Draw sky
        window.draw(skySprite);

        // Draw hearts (top-right)
        for (int i = 0; i < lives; ++i) {
            float x = window.getSize().x
                    - (i + 1) * (heartSize + heartSpacing);
            float y = 10.f;

            heartSprite.setPosition({x, y});
            window.draw(heartSprite);
        }

        // Draw bees
        for (const auto& bee : bees) {
            window.draw(bee.sprite);
        }

        // Draw terrain
        for (auto &tile : terrainRow) {
            window.draw(tile);
        }

        // Draw player sprite
        window.draw(sprite);

        // Draw score
        drawScore(window, textTexture, score, {20.f, 20.f});
        
        window.display();
    }
}

bool loadTexture(sf::Texture& texture, const std::string& path){ //function to load textures
    if (!texture.loadFromFile(path)){
        std::cout << "Failed to load: " << path << "\n";
        return false;
    }
    return true;
}

void clampToScreen(sf::Sprite& sprite, const sf::RenderWindow& window){ //function to keep player on screen
    auto bounds = sprite.getLocalBounds();  // Get the actual local size of the sprite (32x32)
    float halfW = bounds.size.x * 0.5f;     // =16
    float halfH = bounds.size.y * 0.5f;

    float left   = 0 + halfW;
    float right  = window.getSize().x - halfW;
    float top    = 0 + halfH;
    float bottom = window.getSize().y - halfH;

    sf::Vector2f pos = sprite.getPosition();

    if (pos.x < left)  pos.x = left;
    if (pos.x > right) pos.x = right;
    if (pos.y < top)   pos.y = top;
    if (pos.y > bottom)pos.y = bottom;

    sprite.setPosition(pos);
}

    void drawScore(
        sf::RenderWindow& window,
        const sf::Texture& textTexture, 
        int score,
        sf::Vector2f position,
        float scale
    ){
        std::string scoreStr = std::to_string(score);

        sf::Sprite digitSprite(textTexture);
        digitSprite.setScale({scale, scale});

        for (std::size_t i = 0; i < scoreStr.size(); ++i){
            int digit = scoreStr[i] - '0';

            digitSprite.setTextureRect(sf::IntRect{
                { digit * 8, FONT_ROW * 10 },
                { 8, 10 }
            });

            digitSprite.setPosition({
                position.x + i * 8.f * scale,
                position.y
            });

            window.draw(digitSprite);
        }
    }




