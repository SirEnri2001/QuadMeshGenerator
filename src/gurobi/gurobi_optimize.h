#pragma once
#include "../mesh/components.h"
#include "../mesh/display.h"
#include "../mesh/operator.h"
#include "glm.hpp"

namespace quadro {
	class MeshFieldDisplay : public MeshDisplay {
	public:
		void display() override;
	};
	class GurobiSolver : public MeshUserOperator {
	private:
		std::unique_ptr<MeshAttribute<glm::vec3>> vectorField;
		float getArea(const Face* face);
		float weight(const Halfedge* he);
	public:
		GurobiSolver(Mesh* mesh);
		~GurobiSolver();
		void prepare();
		void optimize();
		void operator()();

	};
}