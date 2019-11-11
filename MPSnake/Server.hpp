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
		bool is_ready = false;

		bool operator== (ENetPeer* p) { return peer == p; }
	};

	struct GameData {
		struct Food {
			enet_uint16 x = 0, y = 0;
			bool is_enabled = false;
		};
		enet_uint16 map_width = 50, map_height = 50;
		Food food_list[50];
	}game_data;

	enum GameStatus {
		waiting_for_players,
		starting,
		in_game
	}current_status = waiting_for_players;

	void Start();
	void SendString(std::string data, ENetPeer* peer, enet_uint32 flags); //ENET_PACKET_FLAG

	User * FindClient(ENetPeer * peer);

	void OnConnect(ENetEvent& e);
	void OnDisconnect(ENetEvent& e);
	void OnStart();
	void OnWaitForPlayers();

	void SendUserData(User& user);
	void SendGameData(ENetPeer* peer);
	void SendGameStart(User& user);

	std::vector<User> connected_clients;
	enet_uint16 current_id = 0;

	ENetHost* server = NULL;
};