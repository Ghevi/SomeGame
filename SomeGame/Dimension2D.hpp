#include <SFML/Graphics.hpp>

struct Dimension2D
{
	float width{};
	float height{};

	sf::Vector2f toVector2f() const
	{
		return { width, height };
	}

	sf::Vector2f getCenter() const
	{
		return { width / 2, height / 2 };
	}
};