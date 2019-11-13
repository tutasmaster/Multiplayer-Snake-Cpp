#pragma once

#define ENET_IMPLEMENTATION
#include "enet.h"
#include "SFML/Graphics.hpp"
#include "Serialization.hpp"
#include "Game.hpp"
#include "Message.hpp"
#include <iostream>
#include <vector>

struct Map {
	Map(int w,int h) : width(w), height(h){}
	enet_uint16 width, height;
};

class Client {
public:

	struct NetworkData {
		enet_uint16 id;
	}network_data;

	Client();
	void Start();

	/*GAME_LOGIC*/
	Snake my_snake;
	Snake other_snake;

	std::unique_ptr<Map> map = nullptr;

	/*NETWORKING*/
	int Connect(std::string ip, enet_uint16 port);
	void Disconnect();
	int Handshake();
	void SendString(std::string data, enet_uint32 flags); //ENET_PACKET_FLAG

	void SendReady();
	void SendDirection();

	/*DRAW_MAP*/
	void DrawMap();
	void DrawSnake(Snake * snake);

	sf::RenderWindow render_window;

	ENetHost* client = nullptr;
	ENetPeer* server = nullptr;
	bool connected = false;

};