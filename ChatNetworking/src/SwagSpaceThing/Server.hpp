#pragma once

#include <SFML/Graphics.hpp>

#include "Packet.hpp"

class Server
{
public:
	Server();

	int run();
private:
	void init();
	void update(float dt);
	void render();
private:
	sf::RenderWindow window;

	// Render Elements
	sf::Font font;
	sf::Text title;
	sf::Text tempText;

	// Networking Stuff
	PlayerPacket Ppac;
};