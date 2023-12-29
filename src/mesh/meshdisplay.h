#pragma once
#include "meshcomponents.h"

class MeshDisplay {
	Mesh* mesh;
public:
	std::vector<glm::vec4> vertexBuffer;
	std::vector<ID> indices;

	std::vector<glm::vec4> lineSegment;
	std::vector<glm::vec4> pointScatter;

	glm::vec3 calculateSurfaceNormal(const Face* face);

	MeshDisplay(Mesh* mesh);
	void create();
};