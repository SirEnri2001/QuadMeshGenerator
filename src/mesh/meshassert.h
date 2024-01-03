#pragma once
#include "mesh.h"
#include <string>
#include <exception>


#define error(x) __debugbreak();throw x;
#define validate(x) if(!x) {error("Validate failed");}

class MeshAssert {
	std::string traceback;
public:
	void _error(std::string& msg);
};