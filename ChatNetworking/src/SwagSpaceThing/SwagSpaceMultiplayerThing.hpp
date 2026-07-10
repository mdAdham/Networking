#pragma once

#include <SFML/Graphics.hpp>

#include "Client.hpp"
#include "Server.hpp"

#include <iostream>

int SwagSpaceMultiplayerThing()
{
	char choice;
	
	std::cout << "Server (s) or the Client (c): ";
	std::cin >> choice;

	int retval = 0;

	if (choice == 's')
	{
		Server server;
		retval = server.run();
	}
	else if (choice == 'c')
	{
		Client client;
		retval = client.run();
	}
	else
	{
		std::cout << "Invalid choice. Exiting." << std::endl;
	}

	return retval;
}

/*
	


*/