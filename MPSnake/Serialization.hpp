#pragma once

#include <vector>
#include <iostream>
#include "enet.h"

namespace Serial {
	struct Packet {
		std::vector<char> data;
		long cur = 0; 

		Packet(){}

		Packet(ENetPacket* p) {
			for (auto i = 0; i < p->dataLength; i++)
				data.push_back(p->data[i]);
		}

		Packet& operator<<(unsigned char d) { data.push_back(d); return *this; }
		Packet& operator<<(unsigned short d)	{ data.push_back(d & 0xFF); data.push_back((d & 0xFF00) >> 8); return *this; }
		Packet& operator<<(char d)				{ data.push_back(d); return *this; }
		Packet& operator<<(short d)			{ data.push_back(d & 0xFF); data.push_back((d & 0xFF00) >> 8); return *this; }
		Packet& operator<<(std::string d) { 
			for (auto i = 0; i < d.size(); i++) { 
				data.push_back(d.at(i)); 
			} 
			data.push_back('\0');
			cur += d.size() + 1;
			return *this;
		}

		Packet& operator>>(unsigned char& d)	{ d = data[cur]; cur++; return *this; }
		Packet& operator>>(unsigned short& d)	{ d = data[cur] + (data[cur + 1] << 8); cur += 2; return *this; }
		Packet& operator>>(char& d)			{ d = data[cur]; cur++; return *this; }
		Packet& operator>>(short& d)			{ d = data[cur] + (data[cur + 1] << 8); cur += 2; return *this; }
		Packet& operator>>(std::string& d) {
			while (data[cur] != '\0') {
				d += data[cur];
				cur++;
			}
			return *this;
		}
	};
}