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
				OnGameEnd();
			}
			break;
		case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
			std::cout << "A client has timed out!\n";
			if (current_status == GameStatus::waiting_for_players)
				OnDisconnect(e);
			else {
				FindClient(e.peer)->is_connected = false;
				OnGameEnd();
			}
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
	std::cout << "Enough players have connected!\n";
	for (auto& c : connected_clients) {
		c.is_ready = false;
		c.is_dead = false;
		SendGameData(c.peer);
	}

	current_status = starting;
}

void Server::OnStart() {
	current_status = in_game;


	connected_clients[0].snake = Snake(spawn_points[0].x, spawn_points[0].y, spawn_points[0].dir, 10);
	SendGameStart(connected_clients[0],spawn_points[0], spawn_points[1]);
	connected_clients[1].snake = Snake(spawn_points[1].x, spawn_points[1].y, spawn_points[1].dir, 10);
	SendGameStart(connected_clients[1], spawn_points[1], spawn_points[0]);
	game_clock.restart();

	std::cout << "X: " << spawn_points[0].x << " Y: " << spawn_points[0].y << "\n";
	std::cout << "X: " << spawn_points[1].x << " Y: " << spawn_points[1].y << "\n";
}

void Server::OnTick() {
	connected_clients[0].is_ready = false;
	connected_clients[1].is_ready = false;
	if (!connected_clients[0].is_dead)
		connected_clients[0].is_dead = connected_clients[0].snake.CheckForCollision(connected_clients[1].snake);
	if (!connected_clients[1].is_dead)
		connected_clients[1].is_dead = connected_clients[1].snake.CheckForCollision(connected_clients[0].snake);
	
	if (connected_clients[0].is_dead || connected_clients[1].is_dead) {
		OnGameEnd();
	}
	
	connected_clients[0].snake.OnMove();
	connected_clients[1].snake.OnMove();
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

void Server::SendGameStart(User& user, Spawnpoint s1, Spawnpoint s2) {
	Serial::Packet p;
	p << MESSAGE_TYPE::START_MATCH << s1.x << s1.y << s1.dir << s2.x << s2.y << s2.dir;
	ENetPacket* packet = p.GetENetPacket();
	enet_peer_send(user.peer, 0, packet);
	enet_host_flush(server);
}

void Server::SendPlayerDirection(User& userA, User& userB) {
	Serial::Packet p;
	p << MESSAGE_TYPE::PLAYER_UPDATE << userB.snake.direction << (userA.is_dead ? (char)DEAD : userA.snake.direction);
	ENetPacket* packet = p.GetENetPacket();
	enet_peer_send(userA.peer, 0, packet);
	enet_host_flush(server);
}

void Server::SendGameEnd(User& user)
{
	Serial::Packet p;
	p << MESSAGE_TYPE::GAME_END;
	ENetPacket* packet = p.GetENetPacket();
	enet_peer_send(user.peer, 0, packet);
	enet_host_flush(server);
}

int main() {
	Server server;

	server.Start();
	return 0;
}