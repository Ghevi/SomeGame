#include <iostream>
#include <SFML/Graphics.hpp>

namespace sf
{
	static const sf::Vector2f VectorZero{ 0, 0 };
}


//struct MovementBase
//{
//	virtual sf::Vector2f getVector() const = 0;
//};

struct FixedMovement
{
	sf::CircleShape ProjectileShape{};
	sf::Vector2f TargetPosition{};
	float ProjectileMovementSpeed{};

	FixedMovement(
		sf::CircleShape projectileShape,
		sf::Vector2f targetPosition,
		float projectileMovementSpeed)
		: ProjectileShape(projectileShape),
		TargetPosition(targetPosition),
		ProjectileMovementSpeed(projectileMovementSpeed)
	{ }

	sf::Vector2f getVector(sf::Time deltaTime) const
	{
		auto difference = TargetPosition - ProjectileShape.getPosition();
		return difference == sf::VectorZero ? sf::VectorZero
			: sf::Vector2f(ProjectileMovementSpeed * deltaTime.asSeconds(), difference.angle()); // can't calculate angle of VectorZero, throws Assertion failed
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


constexpr int windowWidth = 800;
constexpr int windowHeight = 600;

constexpr float playerRadius = 50.f;
constexpr float playerSpeed = 200.f;
constexpr float projectileSpeed = 1000.f;

constexpr float enemySpeed = 300.f;

sf::Clock mainClock;
sf::Clock projectileSpawningClock;

sf::Clock fpsClock;
uint16_t fps = 0;

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

	Debugger debugger{ playerRadius / 100 * 10 , sf::Color::Red };
	Player player{ playerRadius, sf::Color::Blue, debugger };

	sf::Font font{ "resources/fonts/Caliban.ttf" };
	sf::Text text(font, "FPS: ", 20);
	text.setFillColor(sf::Color::White);
	text.setPosition(sf::Vector2f{ 10.f, 10.f });

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
				projectileSpeed
			};
			const Projectile projectile{ projectileBlueprint, projectileMovement };
			if (projectile.Movement.getVector(deltaTime) != sf::VectorZero)
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

			projectile.ProjectileShape.move(projectile.Movement.getVector(deltaTime));
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

		if (fpsClock.getElapsedTime().asMilliseconds() >= 1000)
		{
			text.setString("FPS: " + std::to_string(fps));
			fps = 0;
			fpsClock.restart();
		}
		window.draw(text);

		window.display();
		fps++;
	}

	return 0;
}