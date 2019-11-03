#pragma once

#define ENET_IMPLEMENTATION
#include "enet.h"
#include <iostream>
#include "Serialization.hpp"
#include "Message.hpp"

class Server {
public:
	void Start();
	void SendString(std::string data, ENetPeer* peer, enet_uint32 flags); //ENET_PACKET_FLAG
	void SendGameData(ENetPeer* peer);

	enet_uint16 map_width = 50, map_height = 50;

	ENetHost* server = NULL;
};