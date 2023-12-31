#pragma once
#include "../mesh/mesh.h"
#include <string>

class MeshOperator {
protected:
	Mesh* mesh;
public:
	MeshOperator(Mesh* mesh);
	virtual void create();
};

struct MutableComponent {
	union Component
	{
		Vertex* vertex;
		Halfedge* halfedge;
		Face* face;
	};
};

class MeshInteriorOperator : public MeshOperator {
public:
	MeshInteriorOperator(Mesh* mesh);
};