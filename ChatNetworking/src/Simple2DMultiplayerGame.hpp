#pragma once

//This is TCP based app

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <mutex>

using namespace sf;
using namespace std;

#define BIT(x) 1 << x

#define SERVER_LISTENING_PORT 3500

constexpr float TPS = 16.67;  // 16.67 ms -> 1\60 sec

int server();
int client();

int Simple2DMultiplayerGame()
{
	char choice = 's';
	cout << "Server(s) or Client(c)? "; cin >> choice;

	int retval = (choice == 's') ? server() : client();

	return retval;
}


enum PacketHeader : uint32_t
{
	PH_RequestWorld		= BIT(0), // Client -> Server (Called Once)
	PH_UpdateWorld		= BIT(1), // Server -> Client

	PH_Moving = BIT(2), // Client -> Server
};

struct Player
{
	uint32_t id;
	Vector2f position;
	Color color;
};



int server()
{
	vector<pair<unique_ptr<TcpSocket>, Player>> clients;
	vector<uint32_t> disconnectedPlayers;
	static uint32_t playerIDCounter = 0;

	SocketSelector selector;
	TcpListener listener;

	if (listener.listen(SERVER_LISTENING_PORT) != sf::Socket::Status::Done)
	{
		cout << "Server Could not able to Listen on the port " << SERVER_LISTENING_PORT << endl;
	}

	selector.add(listener);

	mutex consoleMutex;

	auto networkingFunc = [&](stop_token st) {
		cout << "\n\tNetworking Thread Started\n";

		while (!st.stop_requested())
		{
			selector.wait(milliseconds(TPS));

			if (selector.isReady(listener))
			{
				auto& client = clients.emplace_back(pair<unique_ptr<TcpSocket>, Player>(make_unique<TcpSocket>(), Player()));

				if (listener.accept(*client.first) == Socket::Status::Done)
				{
					selector.add(*client.first);
					client.second.id = playerIDCounter++;
					client.second.position = Vector2f(0.f, 0.f);
					client.second.color = Color(rand() % 256, rand() % 256, rand() % 256);
					//cout << "New Player Has Connected: " << client->getRemoteAddress().value() << ":" << client->getRemotePort() << endl;
				}
				else
				{
					cout << "Error accepting Player\n";
					clients.pop_back();
				}
			}
			else
			{
				for (auto& player : clients)
				{
					if (selector.isReady(*player.first))
					{
						Packet pac;

						Socket::Status status = player.first->receive(pac);

						if (status == Socket::Status::Done)
						{
							uint32_t header_raw;
							pac >> header_raw;

							if (header_raw & PH_RequestWorld)
							{
								// SHOULD SEND THE NUMBER OF PLAYERS FIRST, THEN THE PLAYERS DATA
								Packet pacToSend;

								int numOfPlayers = clients.size();
								pacToSend << PH_UpdateWorld;
								pacToSend << numOfPlayers;
								
								for (auto& player : clients)
								{
									pacToSend << player.second.id;
									pacToSend << player.second.position.x << player.second.position.y;
									pacToSend << player.second.color.r << player.second.color.g << player.second.color.b << player.second.color.a;
								}

								for (auto& otherPlayer : clients)
								{
									otherPlayer.first->send(pacToSend);
								}
							}
							else if (header_raw & PH_Moving)
							{
								// SHOULD GET THE PLAYER POSITION AND UPDATE IT IN THE SERVER AND UPDATE THE PLAYER POSITION IN THE OTHER PLAYERS

								Vector2f deltaPos;
								pac >> deltaPos.x >> deltaPos.y;

								player.second.position += deltaPos;
							}
						}
						else if (status == Socket::Status::Disconnected)
						{
							cout << "Player disconnected: " << player.first->getRemoteAddress().value() << ":" << player.first->getRemotePort() << endl;

							selector.remove(*player.first);
							disconnectedPlayers.push_back(player.second.id);
						}
						else
						{
							cout << "Client disconnected: " << player.first->getRemoteAddress().value() << ":" << player.first->getRemotePort() << endl;
							player.first->disconnect();
							selector.remove(*player.first);
							clients.erase(
								remove_if(clients.begin(), clients.end(),
									[](const pair<unique_ptr<TcpSocket>, Player>& c)
									{
										return c.first == nullptr;
									}),
								clients.end()
							);
						}
					}
				}
				clients.erase(
					remove_if(clients.begin(), clients.end(),
						[&](const auto& client) {
							return find(disconnectedPlayers.begin(), disconnectedPlayers.end(), client.second.id) != disconnectedPlayers.end();
						}),
					clients.end()
				);

				// BROADCAST THE WORLD STATE TO ALL THE PLAYERS
				Packet pacToSend;
				int numOfPlayers = clients.size();
				pacToSend << PH_UpdateWorld;
				pacToSend << numOfPlayers;
				for (auto& player : clients)
				{
					pacToSend << player.second.id;
					pacToSend << player.second.position.x << player.second.position.y;
					pacToSend << player.second.color.r << player.second.color.g << player.second.color.b << player.second.color.a;
				}

				for (auto& player : clients)
				{
					player.first->send(pacToSend);
				}
			}
		}

		cout << "\n\tNetworking Thread Stoped\n";

	};

	string input;
	

	jthread networkingThread(networkingFunc);
	sleep(milliseconds(100));

	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	do
	{
		{
			lock_guard<mutex> lock(consoleMutex);
			cout << "Enter command: ";
		}
		//cin.clear();
		getline(cin, input);



	} while (input != "quit");

	networkingThread.request_stop();

	return 0;
}

int client()
{
	TcpSocket server;
	server.setBlocking(true);

	server.connect(IpAddress(127, 0, 0, 1), SERVER_LISTENING_PORT);

	unordered_map<uint32_t, Player> players;
	mutex playerMutex;

	auto networkingFunc = [&](stop_token st) {
		cout << "\n\tNetworking Thread Started\n";
		while (!st.stop_requested())
		{
			Packet pac;
			Socket::Status status = server.receive(pac);

			if (status == Socket::Status::Done)
			{
				uint32_t header_raw;
				pac >> header_raw;

				if (header_raw & PH_UpdateWorld)
				{
					int numofPlayers;
					pac >> numofPlayers;

					unordered_map<uint32_t, Player> newPlayers;

					for (int i = 0; i < numofPlayers; i++)
					{
						Player player;
						pac >> player.id;
						pac >> player.position.x >> player.position.y;
						pac >> player.color.r >> player.color.g >> player.color.b >> player.color.a;

						newPlayers[player.id] = player;

					}
					{
						lock_guard<mutex> lock(playerMutex);
						players.swap(newPlayers);
					}
				}
			}
			else if (status == Socket::Status::Disconnected)
			{
				cout << "Server disconnected\n";
				break;
			}
			//sleep(milliseconds(TPS));
		}
		cout << "\n\tNetworking Thread Stoped\n";
	};

	jthread networkingThread(networkingFunc);
	sleep(milliseconds(100));

	RenderWindow window(VideoMode({ 800, 600 }), "Simple 2D Multiplayer Game");
	window.setFramerateLimit(60u);

	while (window.isOpen())
	{
		while (auto event = window.pollEvent())
		{
			if (event->is<Event::Closed>())
				window.close();
		}

		//UPDATE

		Vector2f deltaPos;
		
		if (window.hasFocus())
		{
			if (Keyboard::isKeyPressed(Keyboard::Scancode::W))
				deltaPos.y += -1.5f;
			if (Keyboard::isKeyPressed(Keyboard::Scancode::A))
				deltaPos.x += -1.5f;
			if (Keyboard::isKeyPressed(Keyboard::Scancode::S))
				deltaPos.y += 1.5f;
			if (Keyboard::isKeyPressed(Keyboard::Scancode::D))
				deltaPos.x += 1.5f;

		}

		if (deltaPos != Vector2f(0.f, 0.0f))
		{
			Packet pac;
			pac << PH_Moving;
			pac << deltaPos.x << deltaPos.y;
			server.send(pac);
		}
		

		//RENDER
		window.clear();
		
		{
			lock_guard<mutex> lock(playerMutex);
			for (auto& [id, player] : players)
			{
				CircleShape shape(40.f);
				shape.setOrigin({ 40.f, 40.f });
				shape.setPosition({ player.position.x, player.position.y });
				shape.setFillColor(player.color);

				window.draw(shape);
			}
		}

		window.display();
	}

	networkingThread.request_stop();

	return 0;
}

/*
	SIMPLE GAME MECHANICS
		-SERVER CANT RENDER THINGS
		-CLIENT SHOULD NOT ABLE TO MODIFY GAME VARIABLES

		-SERVER IS RUNNING AT 60 TPS
		-CLIENT IS RUNNING AT 60 FPS

		-SERVER IS RUNNING IN TWO THREADS
			-MAIN THREAD (FOR CMD INPUTS)
				-READS INPUTS FROM THE CMD
				-ALLOWS TO KICK CLIENTS
				-ALLOWS TO SHUTDOWN THE SERVER
			-NETWORKING THREAD
				-NETWORKING THREAD IS RESPONSIBLE FOR SENDING GAME STATE UPDATES TO THE CLIENTS
				-GAME LOGICS IS HANDLED IN THE NETWORKING THREAD

		-CLIENT IS RUNNING IN TWO THREADS
			-MAIN THREAD
			-NETWORKING THREAD

		-WHEN THE CLIENT CONNECTS TO THE SERVER, THE CLIENT SHOULD SEND PH_NewConnection HEADER TO THE SERVER

*/