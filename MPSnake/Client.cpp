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
	return 0;
}

int Client::Connect(std::string ip, enet_uint16 port) {
	if (enet_initialize() != 0)
		return 1;

	ENetAddress address = { 0 };
	enet_address_set_host(&address, ip.c_str());
	address.port = port;

	client = enet_host_create(NULL, 12, 1, 0, 0);
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
	int r = Connect("192.168.1.69", 7777);
	if (r != 0) {
		std::cout << "Failed to connect! ERR:" << r << "\n" ;
		return;
	}
	

	sf::Clock clock;

	while (render_window.isOpen()) {
		sf::Event e;
		while (render_window.pollEvent(e)) {
			if (e.type == sf::Event::Closed){
				Disconnect();
				render_window.close();
			}
		}

		float fps = clock.restart().asSeconds();
		//std::cout << "FPS: " << 1 / fp << "\n";


		render_window.display();
	}
}

int main()
{
	Client client;
	client.Start();
	return 0;
}
