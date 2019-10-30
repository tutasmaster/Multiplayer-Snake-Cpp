#include "Client.hpp"

void Client::Start() {
	if (enet_initialize() != 0) {
		std::cout << "ENET has failed to initialize!\n";
		return;
	}

	ENetAddress address = { 0 };
	enet_address_set_host(&address, "192.168.1.69");
	address.port = 7777;

	client = enet_host_create(NULL /* create a client host */,
		1 /* only allow 1 outgoing connection */,
		2 /* allow up 2 channels to be used, 0 and 1 */,
		0 /* assume any amount of incoming bandwidth */,
		0 /* assume any amount of outgoing bandwidth */);

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

	enet_host_destroy(client);
}

int main()
{
	Client client;
	client.Start();
	return 0;
}
