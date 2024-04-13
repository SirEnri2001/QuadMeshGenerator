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
		std::unique_ptr<MeshAttribute<double>> scalarFunction;
		std::unique_ptr<MeshAttribute<VertexType>> vertexType;
		std::vector<const Vertex*> saddleVertices;
		std::vector<std::unique_ptr<Traceline>> tracelines;
		std::unique_ptr<MeshAttribute<std::vector<Traceline*>>> vertexTracelines;
	public:
		MorseFunction(Mesh* mesh);
		~MorseFunction();
		void extractMorseVertices();
		void extractMorseFunction();
		void extractWaveFunction();
		void cleanUpMorseFunction();
		void extractQuasiDualComplex();
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
	};
}
