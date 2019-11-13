#pragma once

#define ENET_IMPLEMENTATION
#include "enet.h"
#include <iostream>
#include <array>
#include "Serialization.hpp"
#include "Message.hpp"
#include "SFML/System.hpp"
#include "Game.hpp"

class Server {
public:

	struct User {
		ENetPeer* peer = NULL;
		enet_uint16 id = 0;
		bool is_ready = false;
		bool is_dead = false;
		bool is_connected = true;
		bool is_playing = false;

		Snake snake = Snake(1,1,0,10);

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

	struct Spawnpoint {
		unsigned short x = 0, y = 0;
		char dir = 0;
	};
	const std::array<Spawnpoint, MAXIMUM_PLAYERS> spawn_points 
	{ 
		Spawnpoint{1,1  ,EAST}, 
		Spawnpoint{48,6 ,WEST},
		Spawnpoint{1,11 ,EAST},
		Spawnpoint{48,16,WEST},
		Spawnpoint{1,21 ,EAST},
		Spawnpoint{48,26,WEST},
		Spawnpoint{1,31 ,EAST},
		Spawnpoint{48,36,WEST},
		Spawnpoint{1,41 ,EAST},
		Spawnpoint{48,46,WEST}
	};

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
	void OnTick();
	void OnGameEnd();

	void SendUserData(User& user);
	void SendGameData(ENetPeer* peer);
	void SendGameStart(User& user);
	void SendPlayerDirection(User& user);
	void SendGameEnd(User& user);
	void SendDeath(User& userA, User& userB);
	void UpdateDeath(User& user);

	std::vector<User> connected_clients;
	enet_uint16 current_id = 0;

	sf::Clock game_clock;
	const float timestep = 0.1;
	const float wait_time = 10;

	ENetHost* server = NULL;
};