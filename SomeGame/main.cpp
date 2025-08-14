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
	{
	}

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

struct Enemy
{
	sf::CircleShape Shape{};
	int Hp{};
	bool IsEnemyPathingDown = true;

	Enemy(float radius, sf::Color fillColor, int hp)
	{
		Shape = sf::CircleShape{ radius };
		Shape.setFillColor(fillColor);
		Hp = hp;
	}
};

struct Projectile
{
	sf::CircleShape ProjectileShape{};
	FixedMovement Movement;
};

constexpr int windowWidth = 800;
constexpr int windowHeight = 600;
const sf::Vector2f windowSize
{
	static_cast<float>(windowWidth),
	static_cast<float>(windowHeight)
};

constexpr float playerRadius = 50.f;
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

	Debugger debugger{ playerRadius / 100 * 10 , sf::Color::Red };
	Player player{ playerRadius, sf::Color::Blue, debugger };

	sf::Font font{ "resources/fonts/Caliban.ttf" };
	sf::Text text(font, "FPS: ", 20);
	text.setFillColor(sf::Color::White);
	text.setPosition(sf::Vector2f{ 10.f, 10.f });

	std::vector<Projectile> projectiles;
	sf::CircleShape projectileBlueprint{ 5.f };
	projectileBlueprint.setFillColor(sf::Color::White);
	projectileBlueprint.setPosition(player.Shape.getGlobalBounds().getCenter());
	projectileBlueprint.setOrigin(sf::Vector2f{ 5.f, 5.f });

	//std::vector<Enemy> enemies;

	Enemy* enemy = new Enemy{ 15.f, sf::Color::Red, enemyHp };
	enemy->Shape.setPosition(sf::Vector2f{ 400.f, 400.f });
	//enemies.push_back(enemy);

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();
		}

		sf::Time deltaTime = mainClock.restart();

		if (enemy != nullptr)
		{
			auto enemyPosition = enemy->Shape.getPosition();
			const auto enemyVelocity = enemySpeed * deltaTime.asSeconds();
			if (enemyPosition.y + 100 > windowHeight)
				enemy->IsEnemyPathingDown = false;

			if (enemyPosition.y - 100 < 0)
				enemy->IsEnemyPathingDown = true;

			const sf::Vector2f enemyMovement{ 0, enemyVelocity };
			enemy->Shape.move(enemy->IsEnemyPathingDown ? enemyMovement : -enemyMovement);
		}

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

		const auto bounds = player.Shape.getGlobalBounds();
		auto position = player.Shape.getPosition();

		if (position.x + bounds.size.x > windowWidth)
			position.x = windowWidth - bounds.size.x;

		if (position.y + bounds.size.y > windowHeight)
			position.y = windowHeight - bounds.size.y;

		if (position.x < 0)
			position.x = 0;

		if (position.y < 0)
			position.y = 0;

		player.Shape.setPosition(position);

		window.clear(sf::Color::Black);

		window.draw(player.Shape);
		window.draw(player.CenterDebugger.Shape);

		for (auto i = 0; i < projectiles.size(); i++)
		{
			auto& projectile = projectiles[i];

			if (enemy != nullptr && enemy->Shape.getGlobalBounds()
				.findIntersection(projectile.ProjectileShape.getGlobalBounds()))
			{
				enemy->Hp -= 10;
				std::cout << "Enemy hp: " << enemy->Hp << std::endl;
				if (enemy->Hp <= 10)
				{
					enemy = nullptr;
				}

				projectiles.erase(projectiles.begin() + i);
				continue;
			}

			if (const auto isOutOfBound = sf::Vector2fExtensions::isOutOfBounds(
				projectile.ProjectileShape.getPosition(), windowSize))
			{
				projectiles.erase(projectiles.begin() + i);
				continue;
			}

			projectile.ProjectileShape.move(
				projectile.Movement.getVector(deltaTime));

			window.draw(projectile.ProjectileShape);
		}

		if (enemy != nullptr)
		{
			window.draw(enemy->Shape);
		}

		/*for (auto& enemy : enemies)
		{*/
		//}

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