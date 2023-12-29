#pragma once
#include "../mesh/mesh.h"
#include "../mesh/meshoperator.h"
#include <glm.hpp>

class ComponentOperator : public MeshInteriorOperator {
public:
	const Vertex* splitEdge(const Halfedge* he, glm::vec4 pos);
	const Halfedge* swapEdge(const Halfedge* he);
	glm::vec4 bisect(const Halfedge* he1, const Halfedge* he2);
	float angle(glm::vec4, const Halfedge* he);
	float angle(glm::vec4, const Vertex* v, glm::vec4);
	float angle(const Halfedge* he1, const Halfedge* he2);
	int numQuad(const Vertex* v);
	int numQuad(const Halfedge* he);
	bool isQuad(const Face* f);
	int buildQuad(const Halfedge* he1, const Halfedge* he2, const Halfedge* he3);
	int buildQuad(const Halfedge* he1, const Halfedge* he2, const Halfedge* he3, const Halfedge* he4);
	ComponentOperator(Mesh* mesh);
};