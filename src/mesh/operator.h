#pragma once
#include "../mesh/core.h"
#include "../mesh/assert.h"
#include <string>
#include <future>
#include <mutex>

namespace quadro {
	class MeshOperator {
	protected:
		Mesh* mesh;
		MeshDisplay* display;
		MeshAssert* asserts;
	public:
		MeshOperator(Mesh* mesh, MeshDisplay* display = nullptr);
		virtual ~MeshOperator();
		virtual void create();
		void setDisplay(MeshDisplay* display);
		void setAsserts(MeshAssert* ass);
	};

	class MeshInteriorOperator : public MeshOperator {
	public:
		MeshInteriorOperator(Mesh* mesh, MeshDisplay* display = nullptr);
		virtual ~MeshInteriorOperator();
	};

	class MeshUserOperator : public MeshOperator {
		std::unique_ptr<std::thread> mThread;
		std::promise<void> prm;
	public:
		virtual void operator()() = 0;
		MeshUserOperator(Mesh* mesh, MeshDisplay* display = nullptr);
		virtual ~MeshUserOperator();
		std::future<void> async();
	};
}