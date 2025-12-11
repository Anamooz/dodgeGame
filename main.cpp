#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML window");
    window.setFramerateLimit(60);

    // Load texture (sprite idle sheet)
    sf::Texture texture;
    if (!texture.loadFromFile("Assets/Main Characters/Pink Man/Idle (32x32).png")) {
        std::cout << "Failed to load texture\n";
        return -1;
    }

    // ------Sprite setup--------
    sf::Sprite sprite(texture);
    sprite.setTextureRect(sf::IntRect{ {0, 0}, {32, 32} });
    sprite.setOrigin({16.f, 16.f});
    sprite.setPosition({400.f, 300.f});

    // Animation variables
    const int frameCount = 11;
    const int frameWidth = 32;
    const int frameHeight = 32;

    int currentFrame = 0;
    float frameTimer = 0.f;
    float frameSpeed = 0.1f;   // seconds per frame

    sf::Clock clock;

    // ------Movement setup--------
    float speed = 200.f;       // pixels per second
    bool facingLeft = false;

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
                if (moving){
                    frameTimer += dt;
                    if (frameTimer >= frameSpeed){
                        frameTimer = 0.f;
                        currentFrame = (currentFrame + 1) % frameCount;

                        sprite.setTextureRect(sf::IntRect{
                            {currentFrame * frameWidth, 0},
                            {frameWidth, frameHeight}
                        });
                    }
                }
                else
                {
                    // Idle frame (frame 0)
                    currentFrame = 0;
                    sprite.setTextureRect(sf::IntRect{ {0, 0}, {32, 32} });
                }

        // Rendering
        window.clear();
        window.draw(sprite);
        window.display();
    }
}
