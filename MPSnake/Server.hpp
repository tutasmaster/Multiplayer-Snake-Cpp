#pragma once

#define ENET_IMPLEMENTATION
#include "enet.h"
#include <iostream>
#include "Serialization.hpp"
#include "Message.hpp"

class Server {
public:

	struct User {
		ENetPeer* peer = NULL;
		enet_uint16 id = 0;

		bool operator== (ENetPeer* p) { return peer == p; }
	};

	struct GameData {
		struct Food {
			enet_uint16 x = 0, y = 0;
			bool isEnabled = false;
		};
		enet_uint16 map_width = 50, map_height = 50;
		Food food_list[50];
	}game_data;

	void Start();
	void SendString(std::string data, ENetPeer* peer, enet_uint32 flags); //ENET_PACKET_FLAG

	void OnConnect(ENetEvent& e);
	void OnDisconnect(ENetEvent& e);

	void SendGameData(ENetPeer* peer);
	void SendUserData(User& user);

	std::vector<User> connected_clients;
	enet_uint16 current_id = 0;

	ENetHost* server = NULL;
};