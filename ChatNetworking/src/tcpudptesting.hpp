#pragma once

#include <SFML/Network.hpp>
#include <iostream>
#include <string>

using namespace sf;
using namespace std;

int tcpudptesting()
{
	char protocol = 't';
	cout << "TCP(t) or UDP(u)? "; cin >> protocol;
	char choice = 's';
	cout << "Server(s) or Client(c)? "; cin >> choice;

	unsigned short port = 54000;
	cout << "Enter port number: "; cin >> port;

	if (protocol == 't')
	{
		// TCP
		if (choice == 's')
		{
			// Server
			TcpListener listener;
			if (listener.listen(port) != sf::Socket::Status::Done)
				return 1;

			cout << "Server is listening to port " << port << ", waiting for connections... " << endl;

			TcpSocket client;

			if (listener.accept(client) != sf::Socket::Status::Done)
				return 2;

			cout << "Client connected: " << client.getRemoteAddress().value() << endl;

			// Send a message to the connected client
			Packet pac;
			if (client.receive(pac) == Socket::Status::Done)
			{
				string message;
				if (pac >> message)
					cout << "Message received from the client: " << message << endl;
			}
		}
		else
		{
			// Client
			optional<IpAddress> serverIP;
			string ip;

			do
			{
				cout << "Enter IP address to connect: "; cin >> ip;
				if (const auto addresses = sf::Dns::resolve(ip); addresses.has_value() && !addresses->empty())
					serverIP = addresses->front();
			} while (!serverIP.has_value());

			TcpSocket socket;

			if (socket.connect(serverIP.value(), port, seconds(5.f)) == Socket::Status::Done)
			{
				cout << "Connected to server: " << socket.getRemoteAddress().value() << endl;

				// Send a message to the server
				Packet pac;
				pac << "Hello Server, this is from the client!";

				if (socket.send(pac) == Socket::Status::Done)
					cout << "Message sent to the server" << endl;
			}

		}
	}
	else
	{
		// UDP

		if (choice == 's')
		{
			// Server
			UdpSocket server;
			if (server.bind(port) != Socket::Status::Done)
				return 3;

			cout << "Server is listening to port " << port << ", waiting for messages... " << endl;

			Packet pac;
			optional<IpAddress> senderIP;
			unsigned short senderPort;
			server.receive(pac, senderIP, senderPort);
			string message;
			pac >> message;
			cout << "Message received from client " << senderIP.value() << ": " << message << " (Port: " << senderPort << ")" << endl;
			message = "";
			cout << "Enter message to send to the client: "; getline(cin >> ws, message);
			pac.clear();
			pac << message;
			server.send(pac, senderIP.value(), senderPort);
			cout << "Message sent to the client " << senderIP.value() << ": " << "Port: " << senderPort << endl;
			cin.get();
		}
		else
		{
			// Client
			optional<IpAddress> serverIP;
			string ip;
			do
			{
				cout << "Enter IP address to connect: "; cin >> ip;
				if (const auto addresses = sf::Dns::resolve(ip); addresses.has_value() && !addresses->empty())
					serverIP = addresses->front();
			} while (!serverIP.has_value());

			UdpSocket client;
			string message = "Hello, message from the client!";
			Packet pac;
			pac << message;
			client.send(pac, serverIP.value(), port);

			pac.clear();
			client.receive(pac, serverIP, port);

			message = "";
			pac >> message;
			cout << "Message received from server " << serverIP.value() << ": " << message << endl;

			cin.get();
		}
	}
}