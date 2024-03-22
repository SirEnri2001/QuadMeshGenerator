#pragma once
#include "../mesh/core.h"
#include "../mesh/assert.h"
#include <string>
#include <future>
#include <mutex>

namespace quadro {
	/**
	* Base class for MeshOperator. MeshOperator is an object that defines a set of behaivor/algorithm that we want to perforom on the mesh.
	* Expanding operations by pointer or inheritance is recommended.
	* Please use MeshInteriorOperator and MeshUserOperator for actual implementation.
	* @see MeshInteriorOperator
	* @see MeshUserOperator
	*/
	class MeshOperator {
	protected:
		Mesh* mesh;
		MeshDisplay* display;
		MeshAssert* asserts;
	public:
		/**
		* @param mesh Mesh object of this MeshOperator.
		* @param display MeshDisplay object that shows debug information while running the process.
		*/
		MeshOperator(Mesh* mesh, MeshDisplay* display = nullptr);
		/**
		* Destructor.
		*/
		virtual ~MeshOperator();
		/**
		* Should be called before doing operation. Useful when Multiple MeshOperators are related, 
		* i.e. initialize shared_ptr in one MeshUserOperator then initialize them in one time.
		*/
		virtual void create();
		/**
		* @param display MeshOperator::display
		*/
		void setDisplay(MeshDisplay* display);
		/**
		* @param ass MeshOperator::assert
		*/
		void setAsserts(MeshAssert* ass);
		/**
		* @param mesh MeshOperator::mesh
		*/
		void setMesh(Mesh* mesh);
	};

	class MeshInteriorOperator : public MeshOperator {
	public:
		/**
		* Constructor.
		*/
		MeshInteriorOperator(Mesh* mesh, MeshDisplay* display = nullptr);
		/**
		* Destructor.
		*/
		virtual ~MeshInteriorOperator();
	};

	class MeshUserOperator : public MeshOperator {
		std::unique_ptr<std::thread> mThread;
		std::promise<void> prm;
	public:
		/**
		* Defines the actual behavior of this MeshUserOperator.
		*/
		virtual void operator()() = 0;
		/**
		* Constructor.
		*/
		MeshUserOperator(Mesh* mesh, MeshDisplay* display = nullptr);
		/**
		* Destructor.
		*/
		virtual ~MeshUserOperator();
		/**
		* Asynchronized support for multi-thread program. Use this when you call it directly from GUI.
		*/
		std::future<void> async();
	};
}