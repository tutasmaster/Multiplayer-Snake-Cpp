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
	while (true)
	{
		enet_host_service(server, &e, 15);
		switch (e.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			OnConnect(e);
			break;
		case ENET_EVENT_TYPE_RECEIVE:
		{
			User * client = FindClient(e.peer);
			char packet_id;
			if (e.packet == nullptr)
				break;
			Serial::Packet p(e.packet);
			p >> packet_id;
			switch (packet_id) {
			case MESSAGE_TYPE::READY:
				client->is_ready = true;
				break;
			case MESSAGE_TYPE::DIRECTION:
				if(!client->is_dead)
					p >> client->snake.direction;
				break;
			default:
				std::cout << "UNRECOGNIZED MESSAGE RECEIVED!\n";
				break;
			}
			enet_packet_destroy(e.packet);
		}
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			if (current_status == GameStatus::waiting_for_players)
				OnDisconnect(e);
			else {
				FindClient(e.peer)->is_connected = false;
			}
			break;
		case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
			std::cout << "A client has timed out!\n";
			if (current_status == GameStatus::waiting_for_players)
				OnDisconnect(e);
			else {
				User* client = FindClient(e.peer);
				client->is_connected = false;
				client->is_dead = false;
				UpdateDeath(*client);
				OnGameEnd();
			}
			break;
		default:
			break;
		}

		if (current_status == GameStatus::waiting_for_players) {
			if (connected_clients.size() >= 2 && game_clock.getElapsedTime().asSeconds() > wait_time)
				OnWaitForPlayers();
		}
		else if (current_status == GameStatus::starting) {
			bool hasStarted = true;
			for (auto& c : connected_clients) 
				if(!c.is_connected)
					hasStarted = c.is_ready ? c.is_ready : false;
			
			if (hasStarted) OnStart();
		}
		else {
			if (game_clock.getElapsedTime().asSeconds() > timestep) {
				bool player_confirm = true;
				for (auto& c : connected_clients)
					if (c.is_connected)
						player_confirm = c.is_ready ? c.is_ready : false;
				if(player_confirm){
					game_clock.restart();
					OnTick();
				}
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
	std::cout << "Enough players have connected!\n";
	for (auto& c : connected_clients) {
		c.is_ready = false;
		c.is_dead = false;
		c.is_playing = true;
		SendGameData(c.peer);
	}

	game_clock.restart();
	current_status = starting;
}

void Server::OnStart() {
	current_status = in_game;

	for (auto i = 0; i < connected_clients.size(); i++) 
		connected_clients[i].snake = Snake(spawn_points[i].x, spawn_points[i].y, spawn_points[i].dir, 10);

	for (auto& c : connected_clients)
		SendGameStart(c);

	game_clock.restart();
}

void Server::OnTick() {
	for (auto& c : connected_clients) {
		c.is_ready = false;
		if(!c.is_dead)
			for (auto& c1 : connected_clients) 
				if(c.peer != c1.peer)
					c.is_dead = c.snake.CheckForCollision(c1.snake);

		c.snake.OnMove();
	}

	for (auto& c : connected_clients) {
		SendPlayerDirection(c);
	}

	/*if (connected_clients[0].is_dead || connected_clients[1].is_dead) {
		OnGameEnd();
	}*/
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

void Server::OnGameEnd() {
	auto it = connected_clients.begin();
	while (it != connected_clients.end()) {
		auto c = *it;
		if (!c.is_connected)
			it = connected_clients.erase(it);
		else 
			++it;
	}

	for(auto &c : connected_clients)
		SendGameEnd(c);

	current_status = GameStatus::waiting_for_players;
}

void Server::SendGameData(ENetPeer* peer)
{
	Serial::Packet p;
	p << MESSAGE_TYPE::GAME_DATA << game_data.map_width << game_data.map_height << (unsigned short)(connected_clients.size() - 1);
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
	for (auto& c : connected_clients)
		if (c.peer != user.peer) p << c.snake.body[0].x << c.snake.body[0].y << c.snake.direction;
	p << user.snake.body[0].x << user.snake.body[0].y << user.snake.direction;
	ENetPacket* packet = p.GetENetPacket();
	enet_peer_send(user.peer, 0, packet);
	enet_host_flush(server);
}

void Server::SendPlayerDirection(User& user) {
	Serial::Packet p;
	p << MESSAGE_TYPE::PLAYER_UPDATE;
	for (auto& c : connected_clients)
		if(c.peer != user.peer) p << (c.is_playing ? c.snake.direction : (char)DEAD);
	p << (user.is_dead ? (char)DEAD : user.snake.direction);
	ENetPacket* packet = p.GetENetPacket();
	enet_peer_send(user.peer, 0, packet);
	enet_host_flush(server);
}

void Server::SendGameEnd(User& user)
{
	user.is_playing = false;
	Serial::Packet p;
	p << MESSAGE_TYPE::GAME_END;
	ENetPacket* packet = p.GetENetPacket();
	enet_peer_send(user.peer, 0, packet);
	enet_host_flush(server);
}

void Server::UpdateDeath(User& user) {
	int j = 0;
	user.snake.direction = DEAD;
	for (auto i = 0; i < connected_clients.size(); i++) {
		if (connected_clients[i].is_connected) {
			SendDeath(connected_clients[i], user);
		}
	}
}

void Server::SendDeath(User& userA, User& userB) {
	unsigned short id = 0;

	for (auto& c : connected_clients) {
		if (c.peer == userB.peer)
			break;
		if(c.peer != userA.peer)
			id++;
	}
	
	Serial::Packet p;
	p << MESSAGE_TYPE::PLAYER_DEATH << id;
	ENetPacket* packet = p.GetENetPacket();
	enet_peer_send(userA.peer, 0, packet);
	enet_host_flush(server);
}

int main() {
	Server server;

	server.Start();
	return 0;
}