#pragma once
#include "../mesh/mesh.h"
#include <string>
#include <future>
#include <mutex>

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

class MeshUserOperator : public MeshOperator {
	std::unique_ptr<std::thread> mThread; 
	std::promise<void> prm;
public:
	virtual void operator()() = 0;
	MeshUserOperator(Mesh* mesh);
	void setMutex(std::mutex& mu);
	std::future<void> async();
};

class MeshBasicOperator : public MeshOperator {

};