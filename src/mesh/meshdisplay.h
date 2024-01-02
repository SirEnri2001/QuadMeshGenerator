#pragma once
#include "meshcomponents.h"

class MeshDisplay {
	Mesh* mesh;
public:
	std::vector<glm::vec4> vertexBuffer;
	std::vector<ID> indices;

	std::vector<glm::vec4> frameVertexBuffer;
	std::vector<ID> frameIndices;

	std::vector<glm::vec4> heSelect;
	std::vector<ID> heSelectIndices;
	std::vector<glm::vec4> pointScatter;
	int markCount = 0;

	glm::vec3 calculateSurfaceNormal(const Face* face);

	MeshDisplay(Mesh* mesh);
	void create();
	void createFrame();
	void markHalfedge(const Halfedge* he, glm::vec4 color = glm::vec4(1, 0, 1, 1));
};