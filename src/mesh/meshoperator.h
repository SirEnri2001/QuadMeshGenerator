#pragma once
#include "../mesh/mesh.h"

class MeshOperator {
protected:
	Mesh* mesh;
public:
	MeshOperator(Mesh* mesh);
	virtual void create();
};

class MeshInteriorOperator : public MeshOperator {
public:
	MeshInteriorOperator(Mesh* mesh);
};