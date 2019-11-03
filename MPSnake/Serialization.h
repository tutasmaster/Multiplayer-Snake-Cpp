#pragma once

#include <vector>
#include "enet.h"

namespace Serial {
	struct Packet {
		std::vector<char> data;
		int cur = 0;

		void operator<<(unsigned char d)   { data.push_back(d); }
		void operator<<(unsigned short d)  { data.push_back(d & 0xFF); data.push_back((d & 0xFF00) >> 8); }
		void operator<<(char d)			  { data.push_back(d); }
		void operator<<(short d)			  { data.push_back(d & 0xFF); data.push_back((d & 0xFF00) >> 8); }

		void operator>>(unsigned char& d)  { d = data[cur]; cur++; }
		void operator>>(unsigned short& d) { d = data[cur] + (data[cur + 1] << 8); cur += 2; }
		void operator>>(char& d)			  { 
			d = data[cur]; 
			cur++; 
		}
		void operator>>(short& d)		  { d = data[cur] + (data[cur + 1] << 8); cur += 2; }
	};
}