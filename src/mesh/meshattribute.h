#include "mesh.h"

class MeshAttribute {
	Mesh* mesh;
public:
	MeshAttribute(Mesh* mesh);
	double operator[](Component* comp);
	void calculate();
};