#include "Client.hpp"

/*void Client::Start() {
	if (enet_initialize() != 0) {
		std::cout << "ENET has failed to initialize!\n";
		return;
	}

	ENetAddress address = { 0 };
	enet_address_set_host(&address, "192.168.1.69");
	address.port = 7777;

	client = enet_host_create(NULL, 12,1,0,0);

	ENetEvent event;
	ENetPeer* server;
	server = enet_host_connect(client, &address, 2, 0);

	if (enet_host_service(client, &event, 5000) > 0 &&
		event.type == ENET_EVENT_TYPE_CONNECT)
	{
		std::cout << "Connected!\n";
	}
	else {
		std::cout << "Failed to connect!\n";
	}

	ENetPacket* packet = enet_packet_create("packet",
		strlen("packet") + 1,
		ENET_PACKET_FLAG_RELIABLE);

	enet_peer_send(server, 0, packet);
	enet_host_flush(client);

	enet_host_destroy(client);
}*/

Client::Client() : 
	render_window(sf::VideoMode(600,600),"Snake",sf::Style::Close | sf::Style::Titlebar){

}
void Client::SendString(std::string data, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) {
	ENetPacket* packet = enet_packet_create(data.c_str(), data.size(), flags);
	enet_peer_send(server, 0, packet);
	enet_host_flush(client);
	enet_packet_destroy(packet);
}

int Client::Handshake() {
	ENetEvent e;
	while (enet_host_service(client, &e, 5000) >= 0)
	{
		std::cout << e.type << "\n";
		switch (e.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
			std::cout << "The server has sent a message!\n";
			Serial::Packet packet(e.packet);
			char type;
			packet >> type;

			//UPDATE THE PLAYER'S DATA FIRST

			if (type == MESSAGE_TYPE::PLAYER_DATA){
				unsigned short id = 0;
				packet >> id;
				network_data.id = id;
				std::cout << "NETWORK_ID: " << id << "\n";
			}
			else if (type == MESSAGE_TYPE::GAME_DATA) {
				unsigned short width = 0;
				unsigned short height = 0;

				packet >> width >> height;
				map = std::make_unique<Map>(width, height);
				std::cout << "MAP_SIZE: " << width << " , " << height << "\n";
				SendReady();
			}
			else if (type == MESSAGE_TYPE::START_MATCH) {
				unsigned short x1 = 0, x2 = 0, y1 = 0, y2 = 0;
				char dir1 = 0, dir2 = 0;
				packet >> x1 >> y1 >> dir1 >> x2 >> y2 >> dir2;
				std::cout << "Spawnpoint: " << x1 << " : " << y1 << " : " << (int)dir1 << "\n";
				my_snake.SetPosition(x1, y1);
				my_snake.direction = dir1;
				other_snake.SetPosition(x2, y2);
				other_snake.direction = dir2;
				return 0;
			}
			break;
		}
	}
	return 0;
}

void Client::SendReady() {
	Serial::Packet ready_pack;
	ready_pack << MESSAGE_TYPE::READY;
	ENetPacket* packet = ready_pack.GetENetPacket();
	enet_peer_send(server, 0, packet);
	enet_host_flush(client);
}


void Client::SendDirection() {
	Serial::Packet ready_pack;
	ready_pack << MESSAGE_TYPE::DIRECTION << my_snake.direction;
	ENetPacket* packet = ready_pack.GetENetPacket();
	enet_peer_send(server, 0, packet);
	enet_host_flush(client);
}


void Client::DrawMap()
{
	sf::RectangleShape rs(sf::Vector2f((RENDER_WIDTH / map->width)-1, (RENDER_HEIGHT / map->height)-1));
	rs.setOutlineColor(sf::Color::Black);
	rs.setOutlineThickness(1);
	for (int j = 0; j < map->height; j++) {
		for (int i = 0; i < map->width; i++) {
			rs.setPosition(sf::Vector2f(i * (RENDER_WIDTH / map->width), j * (RENDER_HEIGHT / map->height)));
			render_window.draw(rs);
		}
	}
}

void Client::DrawSnake(Snake* snake) {
	sf::RectangleShape rs(sf::Vector2f((RENDER_WIDTH / map->width)-1, (RENDER_HEIGHT / map->height)-1));
	rs.setOutlineColor(sf::Color::Black);
	rs.setFillColor(sf::Color::Green);
	rs.setOutlineThickness(1);
	for (auto n : snake->body) {
		rs.setPosition(n.x * (RENDER_WIDTH / map->width), n.y * (RENDER_HEIGHT / map->height));
		render_window.draw(rs);
	}
}

int Client::Connect(std::string ip, enet_uint16 port) {
	if (enet_initialize() != 0)
		return 1;

	ENetAddress address = { 0 };
	enet_address_set_host(&address, ip.c_str());
	address.port = port;

	client = enet_host_create(NULL, 1, 1, 0, 0);
	server = enet_host_connect(client, &address, 2, 0);

	ENetEvent e;
	if (enet_host_service(client, &e, 5000) > 0 && e.type == ENET_EVENT_TYPE_CONNECT)
		connected = true;
	else 
		return 2;

	enet_host_flush(client);
	
	return Handshake();
}

void Client::Disconnect() {
	if(connected || client != NULL){
		enet_peer_disconnect(server, 0);
		enet_host_flush(client);

		enet_deinitialize();
		enet_host_destroy(client);
	}
}

void Client::Start() {
	int r = Connect("127.0.0.1", 7777);
	if (r != 0) {
		std::cout << "Failed to connect! ERR:" << r << "\n" ;
		return;
	}
	

	sf::Clock clock;
	render_window.setFramerateLimit(30);

	while (render_window.isOpen()) {
		ENetEvent network_event;
		while (enet_host_service(client, &network_event, 0) > 0)
		{
			switch (network_event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				break;
			case ENET_EVENT_TYPE_RECEIVE:
			{
				Serial::Packet packet(network_event.packet);
				char id = 0;
				packet >> id;
				switch (id) {
				case MESSAGE_TYPE::PLAYER_UPDATE:
					packet >> other_snake.direction >> my_snake.direction;
					my_snake.OnMove();
					other_snake.OnMove();
					SendReady();
					break;
				case MESSAGE_TYPE::GAME_END:
					Handshake();
					break;
				default:
					std::cout << "UNRECOGNIZED MESSAGE RECEIVED!\n";
				}
				enet_packet_destroy(network_event.packet);
			}
			break;
			case ENET_EVENT_TYPE_DISCONNECT:
				break;
			case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
				break;
			default:
				break;
			}
		}

		sf::Event e;
		while (render_window.pollEvent(e)) {
			if (e.type == sf::Event::Closed){
				Disconnect();
				render_window.close();
			}
			else if (e.type == sf::Event::KeyPressed) {
				if (e.key.code == sf::Keyboard::Right) {
					if (my_snake.direction != DEAD) {
						my_snake.direction++;
						my_snake.direction = my_snake.direction % 4;
					}
					SendDirection();
				}
				else if (e.key.code == sf::Keyboard::Left) {
					if(my_snake.direction != DEAD){
						my_snake.direction--;
						my_snake.direction = my_snake.direction < 0 ? 3 : my_snake.direction;
					}
					SendDirection();
				}
			}
		}

		float fps = clock.restart().asSeconds();

		//std::cout << "FPS: " << 1 / fp << "\n";
		DrawMap();

		DrawSnake(&other_snake);
		DrawSnake(&my_snake);

		render_window.display();
	}
}

int main()
{
	Client client;
	client.Start();
	return 0;
}
