#pragma once
#include "../mesh/mesh.h"
#include "../mesh/meshoperator.h"
#include <glm.hpp>
#include <vector>
#include <list>

class ComponentOperator : public MeshInteriorOperator {
public:
	glm::vec3 normalVertex(const Vertex* vertex);
	glm::vec4 bisect(const Halfedge* he1, const Halfedge* he1_next);

	float angle(glm::vec3 cp1, const Vertex* pivot, glm::vec3 cp2);
	float angle(const Halfedge* bhe, const Halfedge* bhe_next);
	float angle(glm::vec3 cpoint, const Halfedge* he);

	void deleteVertexMergeFace(Vertex* v);
	void deleteEdgeMergeFace(Halfedge* he); // return 0 : success; return 1 : degenerate

	// Assumes adjacent faces of the edge are all triangles.
	const Vertex* splitEdge(Halfedge*, glm::vec3);
	// Assumes the two adjacent faces of the edge are both triangles.
	Halfedge* swapEdge(Halfedge*);

	void clearFace(std::vector<Halfedge*> heList);
	void clearFace(std::initializer_list<Halfedge*> heList);

	int numQuad(const Halfedge* edge);
	int numQuad(const Halfedge* edge, const Halfedge* edgeSym);
	int numQuad(const Vertex* vert);
	int numTriangles(const Vertex* vert);
	bool isQuad(const Face* face);
	const Vertex* mergeEdge(Vertex* va, Vertex* vb_to_be_deleted);
	void buildQuad(Halfedge* left, Halfedge* bottom, Halfedge* right, Halfedge* top) {
		clearFace({ left,bottom,right,top });
	}
	int faceEdges(const Face* face) {
		int count = 0;
		const Halfedge* he = face->getHalfedge();
		do {
			count++;
		} while (he = he->getNext(), he != face->getHalfedge());
		return count;
	}

	const Halfedge* splitFace(Vertex* v1, Vertex* v2);
	ComponentOperator(Mesh* mesh);
};