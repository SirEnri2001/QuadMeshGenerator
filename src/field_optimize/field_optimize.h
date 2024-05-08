#include "../mesh/operator.h"
#include "../mesh/components.h"

namespace quadro {
	class FieldOptimizer : public MeshUserOperator {
	private:
		std::unique_ptr<MeshAttribute<glm::vec3>> vectorField;
		float getArea(const Face* face);
		float weight(const Halfedge* he);
	public:
		FieldOptimizer(Mesh* mesh);
		~FieldOptimizer();
		void prepare();
		void optimize();
		void operator()();
	};
}