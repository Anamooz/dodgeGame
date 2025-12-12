#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>

bool loadTexture(sf::Texture& texture, const std::string& path);
void clampToScreen(sf::Sprite& sprite, const sf::RenderWindow& window);

int main(){
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Dodge Game");
    window.setFramerateLimit(60);

    sf::Texture idleTexture;
    sf::Texture runTexture;
    sf::Texture terrainTexture;

    if(!loadTexture(idleTexture, "Assets/Main Characters/Pink Man/Idle (32x32).png")){
        return -1;
    }

    if(!loadTexture(runTexture, "Assets/Main Characters/Pink Man/Run (32x32).png")){
        return -1;
    }

    if(!loadTexture(terrainTexture, "Assets/Terrain/Terrain (16x16).png")){
        return -1;
    }  

    // ------Terrain setup--------
    sf::Sprite terrainTile(terrainTexture);
    terrainTile.setTextureRect(sf::IntRect{ {0,0},{16,16} }); 

    const int tileSize = 16;
    int windowWidth = window.getSize().x;
    int windowHeight = window.getSize().y;

    // How many tiles fit horizontally
    int tileCount = windowWidth / tileSize + 1; // +1 in case of rounding
    // --- Pre-create terrain tiles in a vector ---
    std::vector<sf::Sprite> terrainRow;
    terrainRow.reserve(tileCount); // avoids reallocations

    for (int i = 0; i < tileCount; ++i){
        sf::Sprite tile(terrainTexture);

        tile.setTextureRect(sf::IntRect{{128, 0}, {tileSize, tileSize}});

        // Set position using explicit floats to avoid narrowing warnings
        tile.setPosition(sf::Vector2f(
            static_cast<float>(i * tileSize),
            static_cast<float>(windowHeight - tileSize)
        ));

        terrainRow.push_back(tile);
    }

    // ------Sprite setup--------
    sf::Sprite sprite(idleTexture); //start idle texture
    sprite.setTextureRect(sf::IntRect{ {0, 0}, {32, 32} });
    sprite.setOrigin({16.f, 16.f});
    sprite.setPosition({400.f, 300.f});

    // Animation variables
    const int idleFrames = 11; // Number of frames in idle animation
    const int runFrames  = 12; // Number of frames in run animation

    int currentFrame = 0;
    float frameTimer = 0.f;
    float frameSpeed = 0.10f;

    // Keep track of current texture pointer
    const sf::Texture* currentTexture = &idleTexture;

    // ------Movement setup--------
    float speed = 200.f;       // pixels per second
    bool facingLeft = false;


    sf::Clock clock;

    // ------Game loop--------
    while (window.isOpen()){
        while (auto event = window.pollEvent()){
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        float dt = clock.restart().asSeconds();

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

        // --- ANIMATION ---
        frameTimer += dt;

        // --- SWITCH TEXTURE IF STATE CHANGES ---
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

        // -------Rendering---------
        window.clear();

        // Draw terrain
        for (auto &tile : terrainRow){
            window.draw(tile);
        }
        window.draw(sprite);
        window.display();
    }
}

bool loadTexture(sf::Texture& texture, const std::string& path){
    if (!texture.loadFromFile(path)){
        std::cout << "Failed to load: " << path << "\n";
        return false;
    }
    return true;
}

void clampToScreen(sf::Sprite& sprite, const sf::RenderWindow& window){
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



