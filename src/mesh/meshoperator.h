#pragma once
#include "../mesh/mesh.h"
#include <string>
#include <future>

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

class MeshUserOperator : public MeshOperator {
	std::unique_ptr<std::thread> mThread; 
	std::promise<void> prm;
public:
	virtual void operator()() = 0;
	MeshUserOperator(Mesh* mesh);
	std::future<void> async();
};