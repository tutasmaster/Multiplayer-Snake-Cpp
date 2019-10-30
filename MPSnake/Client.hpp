#pragma once

#define ENET_IMPLEMENTATION
#include "enet.h"
#include "SFML/Graphics.hpp"
#include <iostream>

class Client {
public:
	void Start();

	ENetHost* client;

};