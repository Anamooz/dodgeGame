#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Dodge Game");
    window.setFramerateLimit(60);

    // Load idle texture
    sf::Texture idleTexture;
    if (!idleTexture.loadFromFile("Assets/Main Characters/Pink Man/Idle (32x32).png")) {
        std::cout << "Failed to load idle texture\n";
        return -1;
    }

    // Load run texture 
    sf::Texture runTexture;
    if (!runTexture.loadFromFile("Assets/Main Characters/Pink Man/Run (32x32).png")) {
        std::cout << "Failed to load run texture\n";
        return -1;
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

        // Rendering
        window.clear();
        window.draw(sprite);
        window.display();
    }
}
