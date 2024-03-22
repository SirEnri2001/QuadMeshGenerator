#include "../mesh/operator.h"
#include "../mesh/components.h"
namespace quadro {
	class MorseFunction : public MeshUserOperator {
		std::unique_ptr<MeshAttribute<float>> scalarFunction;
	public:
		MorseFunction(Mesh* mesh);
		~MorseFunction();
		void extractMorseFunction();
		void paintMorseFunction();

		void extractWaveFunction();
		void markMorseFunctionPoints();
		void paintWaveFunction();

		void operator()();
	};
}
