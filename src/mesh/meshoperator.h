#include "meshcomponents.h"

class MeshOperator {
	Mesh* mesh;
public:
	MeshOperator(Mesh* mesh);

	bool loadObj(const std::string& filename);
};