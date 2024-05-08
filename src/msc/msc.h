#include "../mesh/operator.h"
#include "../mesh/components.h"
#include <deque>
namespace quadro {
	class MorseFunction : public MeshUserOperator {
		enum VertexType {
			NONE, REGULAR, MAXIMUM, MINIMUM, SADDLE
		};
		struct Traceline {
			const Vertex* end1; // saddle point
			const Vertex* end2; // extreme point
			std::deque<const Halfedge*> line;
			bool toMaximum;
			bool isValid = true;
		};
		struct Patch {
			std::vector<const Vertex*> corners; //starts with the minimum id vertex
			std::vector<Traceline*> tracelines;
			bool operator()(const Patch& p) {
				return
					corners[0]->getId() == p.corners[0]->getId()
					&& corners[1]->getId() == p.corners[1]->getId()
					&& corners[2]->getId() == p.corners[2]->getId()
					&& corners[3]->getId() == p.corners[3]->getId();
			}
		};
		std::unique_ptr<MeshAttribute<double>> scalarFunction;
		std::unique_ptr<MeshAttribute<VertexType>> vertexType;
		std::vector<const Vertex*> saddleVertices;
		std::vector<std::unique_ptr<Traceline>> tracelines;
		std::vector<std::unique_ptr<Patch>> patches;
		std::unique_ptr<MeshAttribute<std::vector<Traceline*>>> vertexTracelines;
		std::unique_ptr<MeshAttribute<int>> transitionFunction;
		
		void addPatch(std::initializer_list<const Vertex*> corners);
	public:
		MorseFunction(Mesh* mesh);
		~MorseFunction();
		void extractMorseVertices();
		void extractMorseFunction();
		void extractWaveFunction();
		void cleanUpMorseFunction();
		void extractQuasiDualComplex();
		void calculatePathTransitionFunction();
		void calculatePatch();
		void paintMorseFunction();
		void markMorseFunctionPoints();
		void paintWaveFunction();
		void smoothScalarFunction(const Vertex* v);
		void removeMorseFunctionVertex(const Vertex* v);
		void operator()();
		double getScalarFunction(const Vertex* v);
		double getGradientScalarFunction(const Halfedge* he);
		VertexType getVertexType(const Vertex* v);
		void setVertexType(Vertex* v, VertexType t);
		Traceline* traceToMaximum(const Halfedge* he);
		Traceline* traceToMinimum(const Halfedge* he);
		Traceline* trace(const Halfedge* he, bool toMaximum);
		Traceline* trace(const Vertex* end1, const Vertex* end2);
		Traceline* getTraceline(const Vertex* v1, const Vertex* v2);
		std::vector<Traceline*> nextTracelines(Traceline* tr, const Vertex* targetV);

	};
}
