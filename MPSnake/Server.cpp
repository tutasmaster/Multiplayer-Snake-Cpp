#include "Server.hpp"

void Server::Start() {

	if (enet_initialize() != 0) {
		std::cout << "ENET has failed to initialize!\n";
		return;
	}

	ENetAddress address = { 0 };

	address.host = ENET_HOST_ANY;
	address.port = 7777;

	ENetHost* server = enet_host_create(&address, 32, 2, 0, 0);

	if (server == NULL) {
		std::cout << "ENET failed to start the server!\n";
		return;
	}

	std::cout << "Started a server...\n";



	ENetEvent event;

	while (enet_host_service(server, &event, 0) >= 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			std::cout << "A client has connected!\n";
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			std::cout << "A client has sent a message!\n";
			enet_packet_destroy(event.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			std::cout << "A client has disconnected!\n";
			break;
		}
	}

	enet_host_destroy(server);
	enet_deinitialize();
}

int main() {
	Server server;
	server.Start();
	return 0;
}