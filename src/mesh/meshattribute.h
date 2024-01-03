#include "mesh.h"

template<typename AttribType>
class MeshAttribute {
	Mesh* mesh;
public:
	MeshAttribute(Mesh* mesh);
	AttribType operator[](Component* comp);
	void calculate();
};