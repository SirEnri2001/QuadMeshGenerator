#pragma once
#include "mesh.h"
#include <string>

class MeshAssert {
	std::string traceback;
public:
	void error(std::string& msg);
};