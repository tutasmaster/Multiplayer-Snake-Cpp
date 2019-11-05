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
			OnConnect(e);
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			std::cout << "A client has sent a message!\n";
			enet_packet_destroy(e.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			OnDisconnect(e);
			break;
		case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
			std::cout << "A client has timed out!\n";
			OnDisconnect(e);
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

void Server::OnConnect(ENetEvent& e)
{
	std::cout << "A client has connected!\n";
	User u;
	u.peer = e.peer;
	u.id = current_id++;
	SendUserData(u);
	connected_clients.push_back(u);
}

void Server::OnDisconnect(ENetEvent& e)
{
	std::cout << "A client has disconnected!\n";

	auto it = std::find(connected_clients.begin(), connected_clients.end(), e.peer);
	connected_clients.erase(it);
}

void Server::SendGameData(ENetPeer* peer)
{
	Serial::Packet p;
	p << MESSAGE_TYPE::GAME_DATA << game_data.map_width << game_data.map_height;
	ENetPacket* packet = p.GetENetPacket();
	enet_peer_send(peer, 0, packet);
	enet_host_flush(server);
}

void Server::SendUserData(User& user) {
	Serial::Packet p;
	p << MESSAGE_TYPE::PLAYER_DATA << (unsigned short)user.id;
	ENetPacket* packet = p.GetENetPacket();
	enet_peer_send(user.peer, 0, packet);
	enet_host_flush(server);
}

int main() {
	Server server;
	server.Start();
	return 0;
}