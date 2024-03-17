#pragma once
#include "components.h"
#include <string>
namespace quadro {
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
}