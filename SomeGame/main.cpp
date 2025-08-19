#include <iostream>
#include <SFML/Graphics.hpp>

namespace sf
{
    static const sf::Vector2f VectorZero{ 0, 0 };

    class Vector2fExtensions
    {
    public:
        static bool isOutOfBounds(const sf::Vector2f& vector, const sf::Vector2f& bounds)
        {
            return vector.x < 0 || vector.x > bounds.x
                || vector.y < 0 || vector.y > bounds.y;
        }
    };
}

constexpr int windowWidth = 800;
constexpr int windowHeight = 600;
const sf::Vector2f windowSize
{
    static_cast<float>(windowWidth),
    static_cast<float>(windowHeight)
};

constexpr float playerRadius = 32.f;
constexpr float playerSpeed = 200.f;
constexpr float projectileSpeed = 1000.f;

constexpr float enemySpeed = 300.f;
constexpr int enemyHp = 100;

sf::Clock mainClock;
sf::Clock projectileSpawningClock;

sf::Clock fpsDrawingClock;
const sf::Time fpsCalculationInterval = sf::milliseconds(500);

int main()
{
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    sf::RenderWindow window(
        sf::VideoMode(sf::Vector2u{ windowWidth, windowHeight }),
        "Some Game",
        sf::Style::Default,
        sf::State::Windowed,
        settings);

    sf::Texture playerTexture;
    const int playerSpriteIndexX = 0;
    const int playerSpriteIndexY = 0;
    if (playerTexture.loadFromFile("assets/player/textures/spritesheet.png") == false)
    {
        std::cout << "Player texture loading failed" << std::endl;
    }
    sf::Sprite playerSprite{ playerTexture };
    playerSprite.setTextureRect(sf::IntRect
        {
            sf::Vector2i{ playerSpriteIndexX * 64, playerSpriteIndexY * 64 },
            sf::Vector2i{ 64, 64 }
        });
    playerSprite.scale(sf::Vector2f(3, 3));

    sf::Texture enemyTexture;
    const int enemySpriteIndexX = 2;
    const int enemySpriteIndexY = 2;
    if (enemyTexture.loadFromFile("assets/enemy/textures/spritesheet.png") == false)
    {
        std::cout << "Player texture loading failed" << std::endl;
    }
    sf::Sprite enemySprite{ enemyTexture };
    enemySprite.setTextureRect(sf::IntRect
        {
            sf::Vector2i{ enemySpriteIndexX * 64, enemySpriteIndexY * 64 },
            sf::Vector2i{ 64, 64 }
        });
    enemySprite.setPosition(sf::Vector2f{ 400, 400 });
    enemySprite.scale(sf::Vector2f(3, 3));

    sf::Font font{ "assets/fonts/Caliban.ttf" };
    sf::Text text(font, "FPS: ", 20);
    text.setFillColor(sf::Color::White);
    text.setPosition(sf::Vector2f{ 10.f, 10.f });

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        sf::Time deltaTime = mainClock.restart();

        sf::Vector2f playerMovement = sf::VectorZero;
        const auto playerVelocity = playerSpeed * deltaTime.asSeconds();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            playerMovement.y -= playerVelocity;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            playerMovement.x -= playerVelocity;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            playerMovement.y += playerVelocity;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            playerMovement.x += playerVelocity;

        playerSprite.move(playerMovement);

        window.clear(sf::Color::Black);

        window.draw(enemySprite);
        window.draw(playerSprite);

        if (fpsDrawingClock.getElapsedTime() >= fpsCalculationInterval)
        {
            fpsDrawingClock.restart();
            float fps = 1.f / deltaTime.asSeconds();
            text.setString("FPS: " + std::to_string(static_cast<int>(fps)));
        }

        window.draw(text);

        window.display();
    }

    return 0;
}