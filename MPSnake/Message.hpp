#pragma once

const enum MESSAGE_TYPE : char {
	GAME_DATA = 0,  //SERVER -> CLIENT
	PLAYER_DATA = 1, //SERVER -> CLIENT
	START_MATCH = 2, //SERVER -> CLIENT
	READY = 3        //CLIENT -> SERVER
};


