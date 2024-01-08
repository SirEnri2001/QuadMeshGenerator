#pragma once
#include "../mesh/mesh.h"
#include <string>
#include <future>
#include <mutex>

class MeshOperator {
protected:
	Mesh* mesh;
	MeshDisplay* display;
public:
	MeshOperator(Mesh* mesh, MeshDisplay* display = nullptr);
	virtual void create();
	void setDisplay(MeshDisplay* display);
};

class MeshInteriorOperator : public MeshOperator {
public:
	MeshInteriorOperator(Mesh* mesh, MeshDisplay* display = nullptr);
};

class MeshUserOperator : public MeshOperator {
	std::unique_ptr<std::thread> mThread; 
	std::promise<void> prm;
public:
	virtual void operator()() = 0;
	MeshUserOperator(Mesh* mesh, MeshDisplay* display = nullptr);
	std::future<void> async();
};

class MeshBasicOperator : public MeshOperator {

};