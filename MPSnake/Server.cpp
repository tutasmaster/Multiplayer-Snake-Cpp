#include "Server.hpp"

void Server::Start() {

	if (enet_initialize() != 0) {
		std::cout << "ENET has failed to initialize!\n";
		return;
	}

	ENetAddress address = { 0 };

	address.host = ENET_HOST_ANY;
	address.port = 7777;

	server = enet_host_create(&address, 32, 2, 0, 0);

	if (server == NULL) {
		std::cout << "ENET failed to start the server!\n";
		return;
	}

	std::cout << "Started a server...\n";



	ENetEvent e;

	while (enet_host_service(server, &e, 0) >= 0)
	{
		switch (e.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			std::cout << "A client has connected!\n";
			SendGameData(e.peer);
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			std::cout << "A client has sent a message!\n";
			enet_packet_destroy(e.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			std::cout << "A client has disconnected!\n";
			break;
		case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
			std::cout << "A client has timed out!\n";
			break;
		default:
			break;
		}
	}

	enet_host_destroy(server);
	enet_deinitialize();
}

void Server::SendString(std::string data,ENetPeer * peer, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) {
	ENetPacket* packet = enet_packet_create(data.c_str(), data.size(), flags);
	enet_peer_send(peer, 0, packet);
	enet_host_flush(server);
	enet_packet_destroy(packet);
}

void Server::SendGameData(ENetPeer* peer)
{
	Serial::Packet p;
	p << MESSAGE_TYPE::GAME_DATA << map_width << map_height;
	ENetPacket* packet = p.GetENetPacket();
	enet_peer_send(peer, 0, packet);
	enet_host_flush(server);
}

int main() {
	Server server;
	server.Start();
	return 0;
}