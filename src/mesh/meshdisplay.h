#pragma once
#include "meshcomponents.h"

class MeshDisplay {
	Mesh* mesh;
public:
	std::vector<glm::vec4> vertexBuffer;
	std::vector<ID> indices;
	std::unordered_map<ID, int> vertexBufferOffset;

	std::vector<glm::vec4> frameVertexBuffer;
	std::vector<ID> frameIndices;

	std::vector<glm::vec4> heSelect;
	std::vector<ID> heSelectIndices;
	std::vector<glm::vec4> pointScatter;
	std::vector<ID> pointIndices;
	int markCount = 0;

	glm::vec3 calculateSurfaceNormal(const Face* face);
	glm::vec3 calculateVertexNormal(const Vertex* vertex);

	MeshDisplay(Mesh* mesh);
	void create();
	void createFrame();
	void markHalfedge(const Halfedge* he, glm::vec4 color = glm::vec4(1, 0, 1, 1));
	void markFace(const Face* face, glm::vec4 color = glm::vec4(1, 0.5, 0.5, 1));
	void markBoundaries();
	void markVertex(const Vertex* vertex);
	void markHalfedgeCycle(const Halfedge* he);
};