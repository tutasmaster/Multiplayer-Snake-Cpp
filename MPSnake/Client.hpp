#pragma once

#define ENET_IMPLEMENTATION
#include "enet.h"
#include "SFML/Graphics.hpp"
#include <iostream>
#include <vector>

struct Map {

};

struct Snake {
	struct Node {
		enet_uint16 x = 0, y = 0;
	};
	enet_uint16 size = 3;
	std::vector<Node> body;
	Snake(enet_uint16 x, enet_uint16 y, enet_uint16 size = 3) : size(size) { 
		for (auto i = 0; i < size; i++) body.push_back({ x,y }); 
	}
};

class Client {
public:
	Client();
	void Start();

	/*GAME_LOGIC*/
	Snake local_snake;
	Snake other_snake;

	/*NETWORKING*/
	int Connect(std::string ip, enet_uint16 port);
	void Disconnect();
	int Handshake();
	void SendString(std::string data, enet_uint32 flags); //ENET_PACKET_FLAG

	sf::RenderWindow render_window;

	ENetHost* client = nullptr;
	ENetPeer* server = nullptr;
	bool connected = false;

};