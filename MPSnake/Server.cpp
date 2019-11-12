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
	bool running = true;
	while (running)
	{
		enet_host_service(server, &e, 0);
		switch (e.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			OnConnect(e);
			break;
		case ENET_EVENT_TYPE_RECEIVE:
		{
			User * client = FindClient(e.peer);
			char packet_id;
			Serial::Packet p(e.packet);
			p >> packet_id;
			switch (packet_id) {
			case MESSAGE_TYPE::READY:
				client->is_ready = true;
				break;
			case MESSAGE_TYPE::DIRECTION:
				p >> client->direction;
				client->is_ready = true;
				break;
			default:
				std::cout << "UNRECOGNIZED MESSAGE RECEIVED!\n";
				break;
			}
		}
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

		if (current_status == GameStatus::waiting_for_players) {
			if (connected_clients.size() >= 2)
				OnWaitForPlayers();
		}
		else if (current_status == GameStatus::starting) {
			bool hasStarted = true;
			for (auto& c : connected_clients) 
				hasStarted = c.is_ready ? c.is_ready : false;
			
			if (hasStarted) OnStart();
		}
		else {
			if (game_clock.getElapsedTime().asSeconds() > timestep && connected_clients[0].is_ready && connected_clients[1].is_ready) {
				game_clock.restart();
				OnTick();
			}
		}
	}

	std::cout << "SERVER TERMINATION/FAILURE!\n";

	enet_host_destroy(server);
	enet_deinitialize();
}

Server::User * Server::FindClient(ENetPeer* peer) {
	auto it = std::find(connected_clients.begin(), connected_clients.end(), peer);
	return it._Ptr;
}

void Server::OnWaitForPlayers() {
	for (auto& c : connected_clients) 
		SendGameData(c.peer);

	current_status = starting;
}

void Server::OnStart() {
	current_status = in_game;

	for (auto& c : connected_clients)
		SendGameStart(c);
	game_clock.restart();
}

void Server::OnTick() {
	connected_clients[0].is_ready = false;
	connected_clients[1].is_ready = false;
	SendPlayerDirection(connected_clients[0], connected_clients[1]);
	SendPlayerDirection(connected_clients[1], connected_clients[0]);
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

void Server::SendGameStart(User& user) {
	Serial::Packet p;
	p << MESSAGE_TYPE::START_MATCH;
	ENetPacket* packet = p.GetENetPacket();
	enet_peer_send(user.peer, 0, packet);
	enet_host_flush(server);
}

void Server::SendPlayerDirection(User& userA, User& userB) {
	Serial::Packet p;
	p << MESSAGE_TYPE::PLAYER_UPDATE << userB.direction << userA.direction;
	ENetPacket* packet = p.GetENetPacket();
	enet_peer_send(userA.peer, 0, packet);
	enet_host_flush(server);
}

int main() {
	Server server;
	server.Start();
	return 0;
}