#include "Server.hpp"

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500

void Server::init()
{
	window.create(sf::VideoMode({ WINDOW_WIDTH, WINDOW_HEIGHT }), "Server", sf::Style::Titlebar | sf::Style::Close);
	window.setFramerateLimit(60);

	title.setString("Server");
	title.setCharacterSize(24);
	title.setFillColor(sf::Color::White);
	title.setPosition({(WINDOW_WIDTH / 2) - (title.getLocalBounds().size.x / 2), 10});

	tempText.setCharacterSize(24);
	tempText.setFillColor(sf::Color::Green);
	tempText.setPosition({ (WINDOW_WIDTH / 2) - (tempText.getLocalBounds().size.x / 2), 80 });
}


Server::Server()
	: font("comic.ttf"), title(font), tempText(font)
{
}

int Server::run()
{
	init();
	sf::Clock cl;

	while (window.isOpen())
	{
		while (auto ev = window.pollEvent())
		{
			if (ev->is<sf::Event::Closed>())
			{
				window.close();
			}

			if (const auto* keypressed = ev->getIf<sf::Event::KeyPressed>())
			{
				if (keypressed->code == sf::Keyboard::Key::Escape)
					window.close();
			}
		}

		float dt = cl.restart().asSeconds();

		// Update
		update(dt);

		// Render
		window.clear();

		render();

		window.display();
	}

	return 0;
}

void Server::update(float dt)
{
	tempText.setPosition({ (WINDOW_WIDTH / 2) - (tempText.getLocalBounds().size.x / 2), 80 });
	tempText.setString(std::format("Player Position: ({0}, {1})\nPlayer Angle: {2}", Ppac.pos.x, Ppac.pos.y, Ppac.angle));

	Ppac.angle += 1;

}

void Server::render()
{
	window.draw(title);
	window.draw(tempText);
}