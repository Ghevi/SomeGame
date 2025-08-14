#include <iostream>
#include <SFML/Graphics.hpp>
#include "Dimension2D.hpp"

constexpr int windowWidth = 800;
constexpr int windowHeight = 600;

constexpr float playerRadius = 50.f;
constexpr float playerSpeed = 200.f;
constexpr float projectileSpeed = 1000.f;

constexpr float enemySpeed = 300.f;

namespace sf
{
	static const sf::Vector2f VectorZero{ 0, 0 };
}

sf::Clock mainClock;
sf::Clock projectileSpawningClock;

//struct MovementBase
//{
//	virtual sf::Vector2f getVector() const = 0;
//};

struct FixedMovement
{
private:
	sf::Vector2f _movement{};

public:
	FixedMovement(
		sf::CircleShape projectileShape,
		sf::Vector2f targetPosition,
		float projectileMovementSpeed)
	{
		auto position = projectileShape.getPosition();
		auto difference = targetPosition - projectileShape.getPosition();
		_movement = difference == sf::VectorZero ? sf::VectorZero
			: sf::Vector2f(projectileMovementSpeed, difference.angle()); // can't calculate angle of VectorZero, throws Assertion failed
	}

	sf::Vector2f getVector() const
	{
		return _movement;
	}
};

struct Debugger
{
	sf::CircleShape Shape{};

	Debugger() {}

	Debugger(float radius, sf::Color fillColor)
	{
		Shape = sf::CircleShape{ radius };
		Shape.setFillColor(fillColor);
		Shape.setOrigin(sf::Vector2f{ radius, radius });
	}
};

struct Player
{
	sf::CircleShape Shape{};
	Debugger CenterDebugger{};

	Player(float radius, sf::Color fillColor, Debugger centerDebugger)
	{
		Shape = sf::CircleShape{ radius };
		Shape.setFillColor(fillColor);
		CenterDebugger = centerDebugger;
		CenterDebugger.Shape.setPosition(
			Shape.getGlobalBounds().getCenter());
	}

	void move(sf::Vector2f movement)
	{
		Shape.move(movement);
		CenterDebugger.Shape.setPosition(
			Shape.getGlobalBounds().getCenter());
	}
};



struct Projectile
{
	sf::CircleShape ProjectileShape{};
	FixedMovement Movement;
};

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

	std::vector<sf::Shape*> shapes;

	Debugger debugger{ 10.f , sf::Color::Red };
	Player player{ playerRadius, sf::Color::Blue, debugger };

	shapes.push_back(&player.Shape);
	shapes.push_back(&player.CenterDebugger.Shape);

	std::vector<Projectile> projectiles;
	sf::CircleShape projectileBlueprint{ 5.f };
	projectileBlueprint.setFillColor(sf::Color::White);
	projectileBlueprint.setPosition(player.Shape.getGlobalBounds().getCenter());
	projectileBlueprint.setOrigin(sf::Vector2f{ 5.f, 5.f });

	sf::CircleShape enemy{ 15.f };
	enemy.setFillColor(sf::Color::Red);
	enemy.setPosition(sf::Vector2f{ 400.f, 400.f });
	bool isEnemyPathingDown = true;

	shapes.push_back(&enemy);

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();
		}

		sf::Time deltaTime = mainClock.restart();

		auto enemyPosition = enemy.getPosition();
		const auto enemyVelocity = enemySpeed * deltaTime.asSeconds();
		if (enemyPosition.y + 100 > windowHeight)
			isEnemyPathingDown = false;

		if (enemyPosition.y - 100 < 0)
			isEnemyPathingDown = true;

		const auto enemyMovement = isEnemyPathingDown ? sf::Vector2f{ 0, enemyVelocity }
		: -sf::Vector2f{ 0, enemyVelocity };
		enemy.move(enemyMovement);

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

		player.move(playerMovement);
		projectileBlueprint.setPosition(
			player.Shape.getGlobalBounds().getCenter());

		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)
			&& projectileSpawningClock.getElapsedTime().asMilliseconds() > 100)
		{
			projectileSpawningClock.restart();

			const auto mousePosition = sf::Mouse::getPosition(window);
			const FixedMovement projectileMovement
			{
				projectileBlueprint,
				static_cast<sf::Vector2f>(mousePosition),
				0.5f //projectileSpeed * deltaTime.asSeconds() // not sure why this causes inconsistent speed 
			};
			const Projectile projectile{ projectileBlueprint, projectileMovement };
			if (projectile.Movement.getVector() != sf::VectorZero)
			{
				projectiles.push_back(projectile);
			}
		}

		enemyPosition = enemy.getPosition();
		for (auto i = 0; i < projectiles.size(); i++)
		{
			auto& projectile = projectiles[i];

			const auto optionalIntersection = enemy
				.getGlobalBounds()
				.findIntersection(projectile.ProjectileShape.getGlobalBounds());
			if (optionalIntersection)
			{
				projectiles.erase(projectiles.begin() + i);
			}

			const auto projectilePosition = projectile.ProjectileShape.getPosition();
			if (projectilePosition.x < 0 || projectilePosition.x > windowWidth
				|| projectilePosition.y < 0 || projectilePosition.y > windowHeight)
			{
				projectiles.erase(projectiles.begin() + i);
				std::cout << "Removed element, projectiles container size is " << projectiles.size() << std::endl;
				continue;
			}

			projectile.ProjectileShape.move(projectile.Movement.getVector());
		}

		for (auto shape : shapes)
		{
			const auto bounds = shape->getGlobalBounds();
			auto position = shape->getPosition();

			if (position.x + bounds.size.x > windowWidth)
				position.x = windowWidth - bounds.size.x;

			if (position.y + bounds.size.y > windowHeight)
				position.y = windowHeight - bounds.size.y;

			if (position.x < 0)
				position.x = 0;

			if (position.y < 0)
				position.y = 0;

			shape->setPosition(position);
		}

		window.clear(sf::Color::Black);

		for (const auto shape : shapes)
		{
			window.draw(*shape);
		}

		for (const auto& projectile : projectiles)
		{
			window.draw(projectile.ProjectileShape);
		}

		window.display();
	}

	return 0;
}