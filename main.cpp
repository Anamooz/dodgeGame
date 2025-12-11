#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML window");

    // Load texture (sprite idle sheet)
    sf::Texture texture;
    if (!texture.loadFromFile("Assets/Main Characters/Pink Man/Idle (32x32).png")) {
        std::cout << "Failed to load texture\n";
        return -1;
    }

    // Sprite setup
    sf::Sprite sprite(texture);
    sprite.setTextureRect(sf::IntRect{ {0, 0}, {32, 32} });
    sprite.setOrigin({16.f, 16.f});
    sprite.setPosition({400.f, 300.f});

    // Animation variables
    const int frameCount = 11;
    const int frameWidth = 32;
    const int frameHeight = 32;

    int currentFrame = 0;
    float frameTime = 0.1f;         // seconds per frame
    float timer = 0.f;

    sf::Clock clock;

    while (window.isOpen()){
        while (auto event = window.pollEvent()){
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        float dt = clock.restart().asSeconds();
        timer += dt;

        // Update animation
        if (timer >= frameTime)
        {
            timer = 0.f;
            currentFrame = (currentFrame + 1) % frameCount;

            sprite.setTextureRect(sf::IntRect{
                {currentFrame * frameWidth, 0},
                {frameWidth, frameHeight}
            });
        }

        // Rendering
        window.clear();
        window.draw(sprite);
        window.display();
    }
}
