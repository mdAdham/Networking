#pragma once
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>

using namespace sf;
using namespace std;

int multipleSocketWithSocketSelector()
{	
	char choice = 's';
	cout << "Server(s) or Client(c)? "; cin >> choice;

	if (choice == 's')
	{
		vector<unique_ptr<TcpSocket>> clients;
		SocketSelector selector;
		TcpListener listener;
		listener.listen(2000);

		selector.add(listener);


		while (true)
		{
			if (selector.wait(milliseconds(10)))
			{

				if (selector.isReady(listener))
				{
					auto& client = clients.emplace_back(make_unique<TcpSocket>());

					if (listener.accept(*client) == Socket::Status::Done)
					{
						selector.add(*client);
						cout << "New Client Has Connected: " << client->getRemoteAddress().value() << ":" << client->getRemotePort() << endl;
					}
					else
					{
						cout << "Error accepting client\n";
					}
				}
				else
				{
					for (auto& client : clients)
					{
						if (selector.isReady(*client))
						{
							Packet pac;
							string message;

							if (client->receive(pac) == Socket::Status::Done)
							{
								pac >> message;
								Packet pacToSend;
								pacToSend << client->getRemoteAddress().value().toString() << client->getRemotePort();
								pacToSend << message;
								for (auto& clientt : clients)
								{
									if (clientt != client)
									{
										clientt->send(pacToSend);
									}
								}

								cout << client->getRemoteAddress().value() << ":" << client->getRemotePort() << " - " << message << "\n";

							}
							else
							{
								cout << "Client disconnected: " << client->getRemoteAddress().value() << ":" << client->getRemotePort() << endl;
								client->disconnect();
								selector.remove(*client);
								clients.erase(
									remove_if(clients.begin(), clients.end(),
										[](const unique_ptr<TcpSocket>& c)
										{
											return c == nullptr;
										}),
									clients.end()
								);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		TcpSocket serverSocketfromClient;

		thread receivingthread([](TcpSocket& socket) {
			while (true)
			{
				sleep(milliseconds(10));

				Packet pac;

				if (socket.receive(pac) == Socket::Status::Done)
				{
					string senderIP;
					unsigned short senderPort;
					string message;
					pac >> senderIP >> senderPort >> message;
					
					cout << senderIP << ":" << senderPort << " - " << message << "\n";
				}
			}
		}, ref(serverSocketfromClient));

		if (serverSocketfromClient.connect(IpAddress(127, 0, 0, 1), 2000) == Socket::Status::Done)
		{
			cout << "Successfully connected to the server: " << serverSocketfromClient.getRemoteAddress().value() << endl;
			cout << "Enter message to send to the server.\n";
			while (true)
			{
				sleep(milliseconds(10));

				string message;
				getline(cin, message);

				if (message == "exit")
				{
					cout << "Disconnecting from the server...\n";
					serverSocketfromClient.disconnect();
					receivingthread.detach();
					
					break;
				}

				Packet pac;
				pac << message;

				if (serverSocketfromClient.send(pac) == Socket::Status::Done)
					continue;
			}
		}

	}



	return 0;
}