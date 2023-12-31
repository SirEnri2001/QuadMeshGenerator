#pragma once
#include "meshcomponents.h"

class MeshIO {
	Mesh* mesh;
public:
	MeshIO(Mesh* mesh);
	~MeshIO();
	bool loadObj(const std::string& filename);
	bool loadM(const std::string& filename);
	bool writeM(const std::string& filename);
	bool writeObj(const std::string& filename);
};