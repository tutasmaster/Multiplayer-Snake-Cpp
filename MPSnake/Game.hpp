#pragma once

#define WIDTH 50
#define HEIGHT 50

#define RENDER_WIDTH 600
#define RENDER_HEIGHT 600

#define EAST 0
#define SOUTH 1
#define WEST 2
#define NORTH 3

struct Snake {
	struct Node {
		enet_uint16 x = 0, y = 0;
	};
	enet_uint16 size = 10;
	std::vector<Node> body;
	char direction = 0;
	void OnMove() {

		for (auto i = body.size() - 1; i >= 1; i--) {
			body[i].x = body[i - 1].x;
			body[i].y = body[i - 1].y;
		}

		switch (direction) {
		case 0:
			body[0].x++;
			break;
		case 1:
			body[0].y++;
			break;
		case 2:
			body[0].x--;
			break;
		case 3:
			body[0].y--;
			break;
		case 4:
			break;
		}

		if (body[0].x > WIDTH)
			body[0].x = WIDTH - 1;
		else if (body[0].x == WIDTH)
			body[0].x = 0;

		if (body[0].y > HEIGHT)
			body[0].y = HEIGHT - 1;
		else if (body[0].y == HEIGHT)
			body[0].y = 0;
	}

	bool CheckForCollision(Snake& snake) {
		for (auto& n : snake.body) {
			if (body[0].x == n.x && body[0].y == n.y) {
				direction = 4;
				return true;
			}
		}
		return false;
	}

	void SetPosition(unsigned short x, unsigned short y) {for (auto& n : body) { n.x = x; n.y = y; }}

	Snake(enet_uint16 x = 1, enet_uint16 y = 1,char dir = 0, enet_uint16 size = 3) : size(size) {
		for (auto i = 0; i < size; i++) body.push_back({ x,y });
	}
};