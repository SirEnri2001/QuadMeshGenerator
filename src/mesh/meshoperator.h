#pragma once
#include "../mesh/mesh.h"
#include <string>
#include <sstream>
#include <boost/stacktrace.hpp>

class MeshOperator {
protected:
	Mesh* mesh;
public:
	MeshOperator(Mesh* mesh);
	virtual void create();
};

struct MutableComponent {
	std::stringstream code;
	union Component
	{
		Vertex* vertex;
		Halfedge* halfedge;
		Face* face;
	};
	MutableComponent() {
		code << boost::stacktrace::stacktrace();
	}
};

class MeshInteriorOperator : public MeshOperator {
public:
	MeshInteriorOperator(Mesh* mesh);
};