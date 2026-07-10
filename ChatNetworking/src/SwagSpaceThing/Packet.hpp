#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

class PlayerPacket
{
public:
	sf::Vector2f pos;
	float angle;

	friend sf::Packet& operator<<(sf::Packet& packet, const PlayerPacket& player)
	{
		return packet << player.pos.x << player.pos.y << player.angle;
	}
	friend sf::Packet& operator>>(sf::Packet& packet, PlayerPacket& player)
	{
		return packet >> player.pos.x >> player.pos.y >> player.angle;
	}
};

